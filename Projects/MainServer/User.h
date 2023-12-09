#pragma once

#include "PacketDefine.h"
#include "type.h"

#include <string>
#include <queue>

class User
{
public:
	User() = default;
	~User() = default;

	void Init(const CKServer::int32 index)
	{
		mIndex = index;
		mPacketQueue = std::queue<CKPacket::messageHeader*>();
	}

	void Clear()
	{
		mIsConfirm = false;
	}

	CKServer::int32 GetNetConnIdx()
	{
		return mIndex;
	}
		
	//TODO SetPacketData, GetPacket 함수를 멀티스레드에 호출하고 있다면 공유변수에 lock을 걸어야 한다
	void SetPacketData(CKPacket::messageHeader* packet)
	{
		mPacketQueue.push(packet);
	}

	CKPacket::messageHeader* GetPacket()
	{
		if (mPacketQueue.empty()) return nullptr;
		CKPacket::messageHeader* temp = mPacketQueue.front();
		mPacketQueue.pop();
		return temp;
	}

private:
	CKServer::int32 mIndex = -1;

	bool mIsConfirm = false;
	std::string mAuthToken;
	std::queue<CKPacket::messageHeader*> mPacketQueue;
};

