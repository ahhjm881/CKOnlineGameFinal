#include "RPing.h"
#include "PacketManager.h"
#include "ckutil/PacketUtil.h"

#include <stdio.h>

RPing::RPing()
{
	m_count = 0;
}

void RPing::RegisterPacket(PacketManager* packetManager)
{
	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::reqPing),
		BIND_CALLBACK(Ping)
	);
}

void RPing::Ping(IManagerProvider* provider, CKPacket::messageHeader* packet)
{
	CKPacket::reqPing* a;
	if (CKPacket::PacketUtil::Deserialize(nullptr, packet->arr().data(), packet->arrsize(), packet->type().data(), &a))
	{
		if(m_count % 10000 == 0)
			printf("ping %d, user: %d\n", m_count, packet->userid());
		m_count++;
	}
}
 