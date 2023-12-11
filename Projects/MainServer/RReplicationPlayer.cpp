#include "RReplicationPlayer.h"
#include "PacketManager.h"
#include "UserManager.h"

using namespace CKPacket;

RReplicationPlayer::RReplicationPlayer()
{
}

void RReplicationPlayer::RegisterPacket(PacketManager* packetManager)
{
	packetManager->RegisterCallback(
		BIND_TYPE(reqPlayerPosition),
		BIND_CALLBACK(Synchronize)
	);

    packetManager->RegisterCallback(
        BIND_TYPE(reqChangePlayerColor),
        BIND_CALLBACK(ChangeColor)
    );

	packetManager->RegisterCallback(
		BIND_TYPE(reqReplicatedPlayerDes),
		BIND_CALLBACK(Des)
	);

	packetManager->RegisterCallback(
		BIND_TYPE(CKPacket::SYSTEM_USER_CONNECT),
		BIND_CALLBACK(Join)
	);
}

void RReplicationPlayer::Synchronize(IManagerProvider* provider, CKPacket::messageHeader* header)
{
	auto req = GetPacket<reqPlayerPosition>(header);

	resPlayerPosition res;
	res.set_index(req->index());
	res.set_x(req->x());
	res.set_y(req->y());
	res.set_z(req->z());

	size_t size;
	char* buf = GetPacked(req->index(), &res, &size);

	if (buf != nullptr)
		provider->GetPacketManager()->BroadcastPacket(req->index(), size, buf, false);
}

void RReplicationPlayer::Des(IManagerProvider* provider, CKPacket::messageHeader* header)
{
}

void RReplicationPlayer::Join(IManagerProvider* provider, CKPacket::messageHeader* header)
{
    resReplicatedPlayerGen genRes;
    genRes.set_index(header->userid());

    size_t size;
    char* buf = GetPacked(header->userid(), &genRes, &size);

    std::vector<User*> users;

    provider->GetUserManager()->GetAllUser(users);
    
    provider->GetPacketManager()->BroadcastPacket(header->userid(), size, buf, false);


    for (auto user : users)
    {
        if (header->userid() == user->GetNetConnIdx())
            continue;

        genRes.set_index(user->GetNetConnIdx());
        size_t size;
        char* buf = GetPacked(header->userid(), &genRes, &size);

        if (buf != nullptr)
        {
            provider->GetPacketManager()->SendPacketFunc(header->userid(), size, buf);
            delete[] buf;
        }
    }

}

void RReplicationPlayer::ChangeColor(IManagerProvider* provider, CKPacket::messageHeader* header)
{
    auto req = GetPacket<reqChangePlayerColor>(header);

    resChangePlayerColor res;
    res.set_index(req->index());
    res.set_r(req->r());
    res.set_g(req->g());
    res.set_b(req->b());
    res.set_a(req->a());

    size_t size;
    char* buf = GetPacked(req->index(), &res, &size);

    if (buf != nullptr)
        provider->GetPacketManager()->BroadcastPacket(req->index(), size, buf, true);
}
