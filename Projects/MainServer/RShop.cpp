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
	* �ڵ鷯 ���. Ư�� ��Ŷ�� ���ŵǸ� �ڵ鷯 �Լ��� ȣ��ǵ��� ��
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
	* reqShopBuy ��Ŷ�� ��� ������ id��
	* ������ ���� ������ ��� �߿� �����Ѵٸ�
	* ���� ó�� �� ������ ������ ���� ������ ���ҽ�Ŵ(��Ŷ���� �䱸�� count ��ŭ)
	* ������ ������ ���� ������ 0 ���ϸ�, �������� ��Ͽ��� ���ܽ�Ŵ 
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
	* ������ ���ŵ� ���� ������ ����� ��� �����鿡�� broadcast��
	* ���� ���� ��û�� �÷��̾��� �κ��丮 ������ ���Ž�����(client�� ����ȭ)
	*/
	User* user = provider->GetUserManager()->GetUserByConnIdx(packet->userid());
	std::get<3>(item) = buyingItemCount;
	user->AddItemCount(item);

	// ���� ������ ��� broadcasting
	SendCurrentShopItemList(packet->userid(), true);
	// ���� ��û�� �÷��̾� �κ��丮 ����
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

	// ���� ������ ����� ��û�� Ŭ���̾�Ʈ���� ����
	SendCurrentShopItemList(packet->userid(), false);
}

void RShop::ResetItem(std::mt19937& gen, std::uniform_int_distribution<>& dis)
{

	sql::Connection* con = nullptr;

	ItemListType itemList;
	itemList.reserve(ITEM_LIST_MAX_COUNT);

	try {

		// db ����
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");

		// ����� ���ν��� ���, ������ id�� ������ ���ڵ� ����
		std::unique_ptr<sql::PreparedStatement> stmt(con->prepareStatement("CALL GetItemsRange(?);"));


		// 0~ITEM_LIST_MAX_COUNT ������ ������ item�� 30�� ��ȸ�Ѵ�.
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

			// resultSet �ڿ� ���� �ڵ�
			// ������ Commands out of sync ���� �߻�
			while (stmt->getMoreResults()) { resultSet.reset(stmt->getResultSet()); }
		}

		// mItemList ������ lock�� ���� ��
		std::lock_guard<std::mutex> guard(mResetMutex);
		// ������ ��ȸ�� ������ ����� ���� ���� ������ ������� ���Ž�Ŵ
		mItemList = itemList;
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	SendCurrentShopItemList(0, true);

	// �ڿ� ����
	if (con)
		delete con;
}

void RShop::UpdateClientInventory(IManagerProvider* provider, std::string userName)
{
	CKPacket::resInventoryItems items;
	ItemListType list;

	/**
	* ������ �κ��丮�� ��� ������ ������ packet�� ���
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
	* �κ��丮 ������ ���� ��Ŷ ����
	*/
	size_t size;
	char* buf = GetPacked(user->GetNetConnIdx(), &items, &size);
	provider->GetPacketManager()->SendPacketFunc(user->GetNetConnIdx(), size, buf);
	delete buf;


}

void RShop::SendCurrentShopItemList(int userid, bool useBroadcast)
{
	CKPacket::resShopItemList senddingList;

	// ���� ������ ����� packet�� ���
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
	* ���� ������ ����� ��û�� client����
	* ������ ��� ����
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
	printf("ResetItemUpdate ����..\n");


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, SAMPLE_ITEM_COUNT);

	while (true)
	{
		// ���� ������ ��� ����.
		// DB���� �������� �����ϰ� ������ �ʱ�ȭ �Ѵ�.
		ResetItem(gen, dis);

		// ���� ������ ��� ������ ���� �ֱ⸶�� ���
		std::this_thread::sleep_for(std::chrono::seconds(RESET_INTERVAL_SEC));
	}
}
