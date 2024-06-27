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
	* �ڵ鷯 ���. Ư�� ��Ŷ�� ���ŵǸ� �ڵ鷯 �Լ��� ȣ��ǵ��� ��
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
	* ������ �κ��丮 ���̺� ������ �õ��ϰ�
	* DB�� �κ��丮 �����͸� ������ �ҷ���
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

		// db ����
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");


		/**
		* DB�� ����� ������ �κ��丮 �����͸� �ҷ���
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


			// resultSet �ڿ� ���� �ڵ�
			// ������ Commands out of sync ���� �߻�
			while (invStmt->getMoreResults()) { resultSet.reset(invStmt->getResultSet()); }
		}

		/**
		* ������ �ҷ��� �κ��丮 �����ʹ� item id�� item count �� ����.
		* ����, DB�� ����� ������ ���� �����Ϳ��� ������ ���� ���� ������
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

				// resultSet �ڿ� ���� �ڵ�
				// ������ Commands out of sync ���� �߻�
				while (ItemStmt->getMoreResults()) { resultSet.reset(ItemStmt->getResultSet()); }
			}
		}
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	// �ڿ� ����
	if (con)
		delete con;

	/**
	* ������ ���������� �ҷ��� ������ ��������
	* ������ �����Ű��, Ŭ���̾�Ʈ�� �ҷ��� ������ ���� ����ȭ��Ŵ
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

		// db ����
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


	// �ڿ� ����
	if (con)
		delete con;
}

void RIenventory::TryCreateInventory(IManagerProvider* provider, int userid)
{
	sql::Connection* con = nullptr;

	std::string userName = provider->GetUserManager()->GetUserByConnIdx(userid)->GetName();

	try {

		// db ����
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


	// �ڿ� ����
	if (con)
		delete con;
}

void RIenventory::SynchronizeDB()
{

	/**
	* ������ �����ϴ� ��� �÷��̾��� �κ��丮 ������
	* DB�� �����Ű�� �۾�
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
			// db ����
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


		// �ڿ� ����
		if (con)
			delete con;
	}


}
