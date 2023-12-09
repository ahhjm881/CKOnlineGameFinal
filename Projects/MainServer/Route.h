#pragma once

#define BIND_CALLBACK(x) [this](IManagerProvider* provider, CKPacket::messageHeader* packet) { this->x(provider, packet); }
#define BIND_TYPE(x) x::GetDescriptor()->full_name().data()

#include "PacketDefine.h"
#include "ManagerProvider.h"
#include "ckutil/PacketUtil.h"
#include "type.h"


namespace
{
	const size_t BUF_SIZE = 4096;
}

class Route
{
public:
	Route(size_t areneaSize = 10000)
	{
		m_arena = new google::protobuf::Arena(new char[areneaSize], areneaSize);
	}

	virtual ~Route()
	{
		delete m_arena;
	}

public:
	virtual void RegisterPacket(class PacketManager* packetManager) = 0;

public:
	template<class T>
	__forceinline T* GetPacket(CKPacket::messageHeader* header)
	{
		T* a;
		if (CKPacket::PacketUtil::Deserialize(m_arena, header->arr().data(), header->arrsize(), header->type().data(), &a))
		{
			return a;
		}

		printf("[Route] deserialize fail, packet type: %s\n", header->type().c_str());
		return nullptr;
	}

	__forceinline  char* GetPacked(CKServer::uint32 userid, google::protobuf::Message* packet, size_t* bufSize)
	{
		char g_serializeBuffer[BUF_SIZE];
		char g_packedBuffer[BUF_SIZE];
		size_t packetSize = 0;
		if (CKPacket::PacketUtil::Serialize(packet, g_serializeBuffer, BUF_SIZE, &packetSize) == false)
		{
			printf("[SendPacket] 직렬화 실패, %s\n", packet->GetDescriptor()->full_name().c_str());
			return nullptr;
		}

		CKPacket::messageHeader header;
		header.set_userid(userid);
		size_t packedSize;
		if (CKPacket::PacketUtil::Pack(&header, g_serializeBuffer, packetSize, packet->GetDescriptor()->full_name().data(), g_packedBuffer, &packedSize, BUF_SIZE) == false)
		{
			printf("[SendPacket] 패킹 실패, %s\n", packet->GetDescriptor()->full_name().c_str());
			return nullptr;
		}

		char* packedBuffer = new char[packedSize];
		std::memcpy(packedBuffer, g_packedBuffer, packedSize);
		*bufSize = packedSize;

		return packedBuffer;
	}

	__forceinline CKPacket::messageHeader* WrappPakcet(CKServer::int32 userid, google::protobuf::Message* packet)
	{
		CKPacket::messageHeader* header = new CKPacket::messageHeader;

		char g_serializeBuffer[BUF_SIZE];
		size_t packetSize = 0;
		if (CKPacket::PacketUtil::Serialize(packet, g_serializeBuffer, BUF_SIZE, &packetSize) == false)
		{
			printf("[SendPacket] 직렬화 실패, %s\n", packet->GetDescriptor()->full_name().c_str());
			return nullptr;
		}

		header->set_userid(userid);
		header->set_arr(g_serializeBuffer);
		header->set_type(packet->GetDescriptor()->full_name().data());

		return header;
	}

private:
	google::protobuf::Arena* m_arena;
};

