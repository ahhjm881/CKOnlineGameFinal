#pragma once
#include "Route.h"

class RPing :
    public Route
{
public:
    RPing();
    void RegisterPacket(PacketManager* packetManager) override;
    void Ping(IManagerProvider* provider, CKPacket::messageHeader* packet);
    
private:
    int m_count;
};

