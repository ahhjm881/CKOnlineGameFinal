#include "RConnection.h"
#include "PacketManager.h"
#include "UserManager.h"
#include "type.h"
#include "ckutil/PacketUtil.h"

void RConnection::RegisterPacket(PacketManager* packetManager)
{
	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::SYSTEM_USER_CONNECT),
		BIND_CALLBACK(Connect)
	);
	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::SYSTEM_USER_DISCONNECT),
		BIND_CALLBACK(Disconnect)
	);
}

void RConnection::Connect(IManagerProvider* provider, CKPacket::messageHeader* header)
{
	printf("[ProcessUserConnect] clientIndex: %d\n", header->userid());
	auto pUser = provider->GetUserManager()->AddUser("", header->userid());

	CKPacket::resPlayerJoin join;
	size_t size;
	auto tosendHeader = GetPacked(header->userid(), &join, &size);

	provider->GetPacketManager()->SendPacketFunc(header->userid(), size, tosendHeader);
}

void RConnection::Disconnect(IManagerProvider* provider, CKPacket::messageHeader* header)
{
	printf("[ProcessUserDisConnect] clientIndex: %d\n", header->userid());

	auto pReqUser = provider->GetUserManager()->GetUserByConnIdx(header->userid());
	provider->GetUserManager()->DeleteUser(pReqUser);
}