#pragma once
#include "Route.h"

class RConnection :
    public Route
{
public:
    virtual void RegisterPacket(class PacketManager* packetManager) override;

private:
    void Connect(IManagerProvider* provider, CKPacket::messageHeader* header);
    void Disconnect(IManagerProvider* provider, CKPacket::messageHeader* header);
};

