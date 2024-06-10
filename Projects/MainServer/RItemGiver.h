#pragma once
#include "Route.h"
#include <jdbc/cppconn/resultset.h>
#include <string>

class RItemGiver :
    public Route
{
public:
    virtual void RegisterPacket(class PacketManager* packetManager) override;

private:
    void AddToInventory(IManagerProvider* provider, CKPacket::messageHeader* packet);
    void RemoveFromInventory(IManagerProvider* provider, CKPacket::messageHeader* packet);
    void GetInventoryItems(IManagerProvider* provider, CKPacket::messageHeader* packet);

    void UpdateData(const std::string& playerName, const std::string& itemName, int count);
    void DeleteData(const std::string& playerName, const std::string& itemName);
    CKPacket::resInventoryItems GetAllItems(const std::string& playerName);

private:
};

