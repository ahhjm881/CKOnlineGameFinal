#pragma once
#include "Route.h"

class RReplicationPlayer :
    public Route
{
public:
    RReplicationPlayer();

public:
    // Route을(를) 통해 상속됨
    void RegisterPacket(PacketManager* packetManager) override;
    void Synchronize(IManagerProvider* provider, CKPacket::messageHeader* header);
    void Des(IManagerProvider* provider, CKPacket::messageHeader* header);
    void Join(IManagerProvider* provider, CKPacket::messageHeader* header);
    void ChangeColor(IManagerProvider* provider, CKPacket::messageHeader* header);
};

