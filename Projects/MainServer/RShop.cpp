#include "RShop.h"
#include "PacketManager.h"
#include "UserManager.h"
#include "ckutil/PacketUtil.h"

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>

RShop::RShop()
	:mPacketManager(nullptr)
{

	mResetThread = std::thread([this]() { ResetItemUpdate(); });
}

void RShop::RegisterPacket(PacketManager* packetManager)
{
	mPacketManager = packetManager;

	/**
	* 핸들러 등록. 특정 패킷이 수신되면 핸들러 함수가 호출되도록 함
	*/

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqShopBuy),
		BIND_CALLBACK(Buy)
	);

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqAddToItem),
		BIND_CALLBACK(Sell)
	);

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqShopItemList),
		BIND_CALLBACK(RequireItemList)
	);
}

void RShop::Buy(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
	CKPacket::reqShopBuy* buyInfo;

	if (CKPacket::PacketUtil::Deserialize(nullptr, packet->arr().data(), packet->arrsize(), packet->type().data(), &buyInfo) == false)
		return;

	int buyingItemId = -1;
	int buyingItemCount = -1;
	ItemType item;

	/**
	* reqShopBuy 패킷에 담긴 아이템 id가
	* 서버의 상점 아이템 목록 중에 존재한다면
	* 구매 처리 및 상점의 아이템 보유 수량을 감소시킴(패킷에서 요구한 count 만큼)
	* 상점의 아이템 보유 수량이 0 이하면, 아이템을 목록에서 제외시킴 
	*/
	{
		std::lock_guard<std::mutex> guard(mResetMutex);
		for(auto iter = mItemList.begin(); iter != mItemList.end(); iter++)
		{
			auto& i = *iter;
			if (std::get<0>(i) == buyInfo->itemid() && std::get<3>(i) >= buyInfo->count())
			{
				buyingItemId = std::get<0>(i);
				buyingItemCount = buyInfo->count();

				item = i;

				std::get<3>(i) -= buyingItemCount;

				if (std::get<3>(i) <= 0)
				{
					mItemList.erase(iter);
				}

				break;
			}
		}
	}

	if (buyingItemId == -1 || buyingItemCount == -1) 
		return;

	/**
	* 위에서 갱신된 상점 아이템 목록을 모든 유저들에게 broadcast함
	* 또한 구매 요청한 플레이어의 인벤토리 정보를 갱신시켜줌(client와 동기화)
	*/
	User* user = provider->GetUserManager()->GetUserByConnIdx(packet->userid());
	std::get<3>(item) = buyingItemCount;
	user->AddItemCount(item);

	// 상점 아이템 목록 broadcasting
	SendCurrentShopItemList(packet->userid(), true);
	// 구매 요청한 플레이어 인벤토리 갱신
	UpdateClientInventory(provider, user->GetName());

}

void RShop::Sell(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
}

void RShop::RequireItemList(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
	CKPacket::reqShopItemList* itemInfo;

	if (CKPacket::PacketUtil::Deserialize(nullptr, packet->arr().data(), packet->arrsize(), packet->type().data(), &itemInfo) == false)
		return;

	// 상점 아이템 목록을 요청한 클라이언트에게 보냄
	SendCurrentShopItemList(packet->userid(), false);
}

void RShop::ResetItem(std::mt19937& gen, std::uniform_int_distribution<>& dis)
{

	sql::Connection* con = nullptr;

	ItemListType itemList;
	itemList.reserve(ITEM_LIST_MAX_COUNT);

	try {

		// db 연결
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");

		// 스토어 프로시저 사용, 아이템 id로 아이템 레코드 쿼리
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("CALL GetItemsRange(?);"));


		// 0~ITEM_LIST_MAX_COUNT 범위의 랜덤한 item을 30개 조회한다.
		for (int i = 0; i < ITEM_LIST_MAX_COUNT; i++)
		{
			stmt->setInt(1, dis(gen));
			std::unique_ptr<sql::ResultSet>resultSet(stmt->executeQuery());

			while (resultSet->next())
			{
				ItemType item(
					resultSet->getInt("item_id"),
					resultSet->getString("item_name"),
					resultSet->getString("item_desc"),
					DEFAULT_ITEM_COUNT
				);

				itemList.push_back(item);
			}

			// resultSet 자원 해제 코드
			// 없으면 Commands out of sync 에러 발생
			while (stmt->getMoreResults()) { resultSet.reset(stmt->getResultSet()); }
		}

		// mItemList 접근은 lock을 얻어야 함
		std::lock_guard<std::mutex> guard(mResetMutex);
		// 위에서 조회한 아이템 목록을 현재 상점 아이템 목록으로 갱신시킴
		mItemList = itemList;
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	SendCurrentShopItemList(0, true);

	// 자원 해제
	if (con)
		delete con;
}

void RShop::UpdateClientInventory(IManagerProvider* provider, std::string userName)
{
	CKPacket::resInventoryItems items;
	ItemListType list;

	/**
	* 유저의 인벤토리의 모든 아이템 정보를 packet에 담기
	*/
	User* user = provider->GetUserManager()->GetUserByName(userName);
	if (user == nullptr) return;
	
	user->GetItemAll(list);

	for (auto& i : list)
	{
		auto item = items.add_items();

		item->set_itemid(std::get<0>(i));
		item->set_itemname(std::get<1>(i));
		item->set_itemdesc(std::get<2>(i));
		item->set_count(std::get<3>(i));
	}


	/**
	* 인벤토리 정보를 담은 패킷 전송
	*/
	size_t size;
	char* buf = GetPacked(user->GetNetConnIdx(), &items, &size);
	provider->GetPacketManager()->SendPacketFunc(user->GetNetConnIdx(), size, buf);
	delete buf;


}

void RShop::SendCurrentShopItemList(int userid, bool useBroadcast)
{
	CKPacket::resShopItemList senddingList;

	// 상점 아이템 목록을 packet에 담기
	{
		std::lock_guard<std::mutex> guard(mResetMutex);
		for (auto& i : mItemList)
		{
			auto info = senddingList.add_items();

			info->set_itemid(std::get<0>(i));
			info->set_itemname(std::get<1>(i));
			info->set_itemdesc(std::get<2>(i));
			info->set_itemcount(std::get<3>(i));

		}
	}

	/**
	* 상점 아이템 목록을 요청한 client에게
	* 아이템 목록 전송
	*/
	size_t size;
	char* arr = GetPacked(userid, &senddingList, &size);

	if (useBroadcast)
	{
		mPacketManager->BroadcastPacket(userid, size, arr, true);
	}
	else
	{
		mPacketManager->SendPacketFunc(userid, size, arr);
	}

	delete arr;
}

void RShop::ResetItemUpdate()
{
	printf("ResetItemUpdate 시작..\n");


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, SAMPLE_ITEM_COUNT);

	while (true)
	{
		// 상점 아이템 목록 리셋.
		// DB에서 아이템을 랜덤하게 가져와 초기화 한다.
		ResetItem(gen, dis);

		// 상점 아이템 목록 리셋은 일정 주기마다 대기
		std::this_thread::sleep_for(std::chrono::seconds(RESET_INTERVAL_SEC));
	}
}
