#include "RItemGiver.h"
#include "PacketManager.h"
#include "ckutil/PacketUtil.h"

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>

#include <sstream>
#include <iostream>

#include <stdio.h>

void RItemGiver::RegisterPacket(PacketManager* packetManager)
{
	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqAddToItem),
		BIND_CALLBACK(AddToInventory)
	);

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqRemoveFromItem),
		BIND_CALLBACK(RemoveFromInventory)
	);

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqInventoryItems),
		BIND_CALLBACK(GetInventoryItems)
	);
}

void RItemGiver::AddToInventory(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
	CKPacket::reqAddToItem* itemInfo;

	if (CKPacket::PacketUtil::Deserialize(nullptr, packet->arr().data(), packet->arrsize(), packet->type().data(), &itemInfo) == false)
		return;

	auto info = itemInfo->item();

	UpdateData(
		info.name(),
		info.itemname(),
		info.count()
		);

}

void RItemGiver::RemoveFromInventory(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
	CKPacket::reqRemoveFromItem* itemInfo;

	if (CKPacket::PacketUtil::Deserialize(nullptr, packet->arr().data(), packet->arrsize(), packet->type().data(), &itemInfo) == false)
		return;

	auto info = itemInfo->item();

	DeleteData(info.name(), info.itemname());
}

void RItemGiver::GetInventoryItems(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
	CKPacket::reqInventoryItems* itemInfo;

	if (CKPacket::PacketUtil::Deserialize(nullptr, packet->arr().data(), packet->arrsize(), packet->type().data(), &itemInfo) == false)
		return;

	CKPacket::resInventoryItems items = GetAllItems(itemInfo->name());
	size_t bufSize;
	auto tosendHeader = GetPacked(packet->userid(), &items, &bufSize);

	provider->GetPacketManager()->SendPacketFunc(packet->userid(), bufSize, tosendHeader);
}

void RItemGiver::UpdateData(const std::string& playerName, const std::string& itemName, int count)
{
	sql::Statement* stmt = nullptr;
	sql::Connection* con = nullptr;

	try {
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");

		stmt = con->createStatement();

		std::ostringstream oss;
		oss << "CALL UpdateData(\"" << playerName << "\", \"" << itemName << "\", " << count << ");";

		std::string formattedString = oss.str();

		stmt->execute(formattedString);
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	// 자원 해제
	if (stmt)
		delete stmt;

	if (con)
		delete con;
}

void RItemGiver::DeleteData(const std::string& playerName, const std::string& itemName)
{

	sql::Statement* stmt = nullptr;
	sql::Connection* con = nullptr;

	try {
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");

		stmt = con->createStatement();

		std::ostringstream oss;
		oss << "delete from ckdb.inventory where name = \"" << playerName << "\" and item_name = \"" << itemName << "\";";

		std::string formattedString = oss.str();

		stmt->execute(formattedString);
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}


	// 자원 해제
	if (stmt)
		delete stmt;

	if (con)
		delete con;
}

CKPacket::resInventoryItems RItemGiver::GetAllItems(const std::string& playerName)
{
	sql::Statement* stmt = nullptr;
	sql::Connection* con = nullptr;
	sql::ResultSet* result = nullptr;
	CKPacket::resInventoryItems items;

	try {
		auto driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "admin");
		con->setSchema("ckdb");

		stmt = con->createStatement();

		std::ostringstream oss;
		oss << "\
select name, item_name, item_count \
from ckdb.inventory \
where name = \""\
<< playerName << 
"\";";

		std::string formattedString = oss.str();

		result = stmt->executeQuery(formattedString);
	}
	catch (std::exception& e)
	{
		printf("db fail\n%s\n", e.what());
	}

	if (result)
	{
		while (result->next())
		{
			auto item = items.add_items();
			item->set_name(result->getString("name"));
			item->set_itemname(result->getString("item_name"));
			item->set_count(result->getUInt("item_count"));
		}
	}


	// 자원 해제
	if (stmt)
		delete stmt;

	if (con)
		delete con;

	if (result)
		delete result;

	return items;
}
