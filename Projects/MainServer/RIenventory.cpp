#include "RIenventory.h"
#include "PacketManager.h"
#include "UserManager.h"
#include "ckutil/PacketUtil.h"

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>

#include <sstream>
#include <iostream>

#include <stdio.h>

RIenventory::RIenventory()
	: mProvider(nullptr)
{
	mSynchroizeThread = std::thread([this]() { SynchronizeDB(); });
}

void RIenventory::RegisterPacket(PacketManager* packetManager)
{
	/**
	* 핸들러 등록. 특정 패킷이 수신되면 핸들러 함수가 호출되도록 함
	*/

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqNotifyPlayerName),
		BIND_CALLBACK(Connect));

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::SYSTEM_USER_DISCONNECT),
		BIND_CALLBACK(Disconnect));
}

void RIenventory::Connect(IManagerProvider* provider, CKPacket::messageHeader* header)
{
	mProvider = provider;

	CKPacket::reqNotifyPlayerName* packet;

	if (CKPacket::PacketUtil::Deserialize(nullptr, header->arr().data(), header->arrsize(), header->type().data(), &packet) == false)
		return;

	provider->GetUserManager()->GetUserByConnIdx(header->userid())->SetName(packet->name());

	delete packet;

	/**
	* 유저의 인벤토리 테이블 생성을 시도하고
	* DB의 인벤토리 데이터를 서버로 불러옴
	*/
	TryCreateInventory(provider, header->userid());
	UpdateInventoryServerData(provider, header->userid());
}

void RIenventory::Disconnect(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
	UpdateInventoryDB(provider, packet->userid());
}

void RIenventory::UpdateInventoryServerData(IManagerProvider* provider, int userid)
{

	sql::Connection* con = nullptr;

	std::string userName = provider->GetUserManager()->GetUserByConnIdx(userid)->GetName();
	ItemListType itemList;

	try {

		// db 연결
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");


		/**
		* DB에 저장된 유저의 인벤토리 데이터를 불러옴
		*/
		{
			std::unique_ptr<sql::PreparedStatement> invStmt(con->prepareStatement("CALL GetInventoryItemList(?);"));
			invStmt->setString(1, userName);
			std::unique_ptr<sql::ResultSet>resultSet(invStmt->executeQuery());

			while (resultSet->next())
			{
				ItemType item(
					resultSet->getInt("item_id"),
					"",
					"",
					resultSet->getInt("item_count")
				);

				itemList.push_back(item);
			}


			// resultSet 자원 해제 코드
			// 없으면 Commands out of sync 에러 발생
			while (invStmt->getMoreResults()) { resultSet.reset(invStmt->getResultSet()); }
		}

		/**
		* 위에서 불러온 인벤토리 데이터는 item id와 item count 만 존재.
		* 따라서, DB에 저장된 아이템 원본 데이터에서 아이템 설명 등을 쿼리함
		*/
		{
			std::unique_ptr<sql::PreparedStatement> ItemStmt(con->prepareStatement("CALL GetItemsRange(?);"));
			for (auto& i : itemList)
			{
				ItemStmt->setInt(1, std::get<0>(i));
				std::unique_ptr<sql::ResultSet>resultSet(ItemStmt->executeQuery());

				while (resultSet->next())
				{
					i = ItemType(
						resultSet->getInt("item_id"),
						resultSet->getString("item_name"),
						resultSet->getString("item_desc"),
						std::get<3>(i)
					);
				}

				// resultSet 자원 해제 코드
				// 없으면 Commands out of sync 에러 발생
				while (ItemStmt->getMoreResults()) { resultSet.reset(ItemStmt->getResultSet()); }
			}
		}
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	// 자원 해제
	if (con)
		delete con;

	/**
	* 위에서 최종적으로 불러온 아이템 정보들을
	* 서버에 저장시키고, 클라이언트에 불러온 정보를 보내 동기화시킴
	*/
	User* user = provider->GetUserManager()->GetUserByConnIdx(userid);
	user->ClearItem();

	CKPacket::resInventoryItems senddingItems;

	for (auto& i : itemList)
	{
		user->AddItemCount(i);

		auto item = senddingItems.add_items();

		item->set_itemid(std::get<0>(i));
		item->set_itemname(std::get<1>(i));
		item->set_itemdesc(std::get<2>(i));
		item->set_count(std::get<3>(i));
	}

	size_t size;
	char* buf = GetPacked(userid, &senddingItems, &size);
	provider->GetPacketManager()->SendPacketFunc(userid, size, buf);
	delete buf;
}

void RIenventory::UpdateInventoryDB(IManagerProvider* provider, int userid)
{
	sql::Connection* con = nullptr;

	std::string userName = provider->GetUserManager()->GetUserByConnIdx(userid)->GetName();
	ItemListType items;
	provider->GetUserManager()->GetUserByConnIdx(userid)->GetItemAll(items);

	ItemListType itemList;

	try {

		// db 연결
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");

		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("CALL InventoryUpdate(?, ?, ?, true)"));

		for (auto& i : items)
		{
			stmt->setString(1, userName);
			stmt->setInt(2, std::get<0>(i));
			stmt->setInt(3, std::get<3>(i));
			stmt->execute();
		}
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	// 자원 해제
	if (con)
		delete con;
}

void RIenventory::TryCreateInventory(IManagerProvider* provider, int userid)
{
	sql::Connection* con = nullptr;

	std::string userName = provider->GetUserManager()->GetUserByConnIdx(userid)->GetName();

	try {

		// db 연결
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");

		std::unique_ptr<sql::Statement> stmt(con->createStatement());
		std::ostringstream oss;
		oss << "CALL TryCreateInventoryTable(\"" << userName << "\");";
		stmt->execute(oss.str());
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	// 자원 해제
	if (con)
		delete con;
}

void RIenventory::SynchronizeDB()
{

	/**
	* 서버에 존재하는 모든 플레이어의 인벤토리 정보를
	* DB에 저장시키는 작업
	*/
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(SYNC_DB_INTERVAL_SEC));


		if (mProvider == nullptr)
			continue;

		sql::Connection* con = nullptr;
		std::vector<User*> users;
		mProvider->GetUserManager()->GetAllUser(users);

		try {
			// db 연결
			auto driver = get_driver_instance();
			con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
			con->setSchema("ckdb");

			std::unique_ptr<sql::PreparedStatement> stmt(
				con->prepareStatement("CALL InventoryUpdate(?, ?, ?, true);"));

			for (auto user : users)
			{
				UpdateInventoryDB(mProvider, user->GetNetConnIdx());
			}
		}
		catch (std::exception& e)
		{
			printf("db fail\n%s\n", e.what());
		}


		// 자원 해제
		if (con)
			delete con;
	}


}
