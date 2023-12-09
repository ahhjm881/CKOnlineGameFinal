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
		
	//TODO SetPacketData, GetPacket �Լ��� ��Ƽ�����忡 ȣ���ϰ� �ִٸ� ���������� lock�� �ɾ�� �Ѵ�
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

