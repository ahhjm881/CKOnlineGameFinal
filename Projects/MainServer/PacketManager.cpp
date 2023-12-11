#include <utility>
#include <cstring>
#include <strsafe.h>


#include "UserManager.h"
#include "PacketManager.h"
#include "RedisManager.h"
#include "ckutil/PacketUtil.h"
#include "Route.h"
#include "PacketDefine.h"

using namespace CKPacket;

#define PACKET_NAME(x) x::GetDescriptor()->full_name()

void PacketManager::Init(const CKServer::int32 maxClient_)
{
	mRecvFuntionDictionary = std::unordered_map<PACKET_KEY, ROUTE_CONTAINER>();
	CreateCompent(maxClient_);


	mInjector.RouteInit(*this);
}


void PacketManager::BroadcastPacket(CKServer::int32 sender, CKServer::int32 size, char* buf, bool includingSender)
{
	std::vector<User*> users;
	mUserManager->GetAllUser(users);

	for (auto i : users)
	{
		if (includingSender == false && i->GetNetConnIdx() == sender) continue;
		SendPacketFunc(i->GetNetConnIdx(), size, buf);
	}
}

void PacketManager::RegisterCallback(const PACKET_KEY& key, PROCESS_RECV_PACKET_FUNCTION callback)
{
	auto iter = mRecvFuntionDictionary.find(key);
	if (iter != mRecvFuntionDictionary.end())
	{
		ROUTE_CONTAINER& container = iter->second;
		container.push_back(callback);
	}
	else
	{
		ROUTE_CONTAINER container;
		container.push_back(callback);
		mRecvFuntionDictionary.insert(make_pair(key, std::move(container)));
	}
}

void PacketManager::CreateCompent(const CKServer::int32 maxClient_)
{
	mUserManager = new UserManager;
	mUserManager->Init(maxClient_);


	mRedisMgr = new RedisManager;
}

bool PacketManager::Run()
{	
	//if (mRedisMgr->Run("127.0.0.1", 6379, 1) == false)
	//{
	//	return false;
	//}

	//이 부분을 패킷 처리 부분으로 이동 시킨다.
	mIsRunProcessThread = true;
	mProcessThread = std::thread([this]() { ProcessPacket(); });

	return true;
}

void PacketManager::End()
{
	mRedisMgr->End();

	mIsRunProcessThread = false;

	if (mProcessThread.joinable())
	{
		mProcessThread.join();
	}
}

void PacketManager::ReceivePacketData(const CKServer::int32 clientIndex_, CKPacket::messageHeader* packet)
{
	std::lock_guard<std::mutex> guard(mLock);
	packet->set_userid(clientIndex_);
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);
	pUser->SetPacketData(packet);

	mInComingPacketUserIndex.push_back(clientIndex_);
}

CKPacket::messageHeader* PacketManager::DequePacketData()
{
	std::lock_guard<std::mutex> guard(mLock);
	CKServer::int32 userIndex = 0;

	{
		if (mInComingPacketUserIndex.empty())
		{
			return nullptr;
		}

		userIndex = mInComingPacketUserIndex.front();
		mInComingPacketUserIndex.pop_front();
	}

	auto pUser = mUserManager->GetUserByConnIdx(userIndex);
	auto packetData = pUser->GetPacket();

	if (packetData == nullptr)
	{
		return nullptr;
	}

	packetData->set_userid(userIndex);
	return packetData;
}

void PacketManager::PushSystemPacket(CKPacket::messageHeader* packet_)
{
	std::lock_guard<std::mutex> guard(mLock);
	mSystemPacketQueue.push_back(packet_);
}

CKPacket::messageHeader* PacketManager::DequeSystemPacketData()
{

	std::lock_guard<std::mutex> guard(mLock);
	if (mSystemPacketQueue.empty())
	{
		return nullptr;
	}

	auto packetData = mSystemPacketQueue.front();
	mSystemPacketQueue.pop_front();

	return packetData;
}


void PacketManager::ProcessPacket()
{
	while (mIsRunProcessThread)
	{
		bool isIdle = true;
		CKPacket::messageHeader* packetData = nullptr;
		if (packetData = DequePacketData())
		{ 
			if (packetData != nullptr && CKPacket::PacketUtil::IsPacket(*packetData, SystemEnd::GetDescriptor()->full_name()) == false)
			{
				isIdle = false;
				ProcessRecvPacket(packetData);
			}
		}

		if (packetData = DequeSystemPacketData())
		{
			if (packetData != nullptr)
			{
				isIdle = false;
				ProcessRecvPacket(packetData);

				delete packetData;
			}

		}

		if (auto task = mRedisMgr->TakeResponseTask(); task.TaskID != RedisTaskID::INVALID)
		{
			isIdle = false;
			//ProcessRecvPacket(task.UserIndex, (UINT16)task.TaskID, task.DataSize, task.pData);
			task.Release();
		}

		if(isIdle)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void PacketManager::ProcessRecvPacket(CKPacket::messageHeader* packet)
{
	auto iter = mRecvFuntionDictionary.find(packet->type());
	if (iter != mRecvFuntionDictionary.end())
	{
		ROUTE_CONTAINER& container = iter->second;

		for (auto i : container)
		{
			(i)(this, packet);
		}
	}

}