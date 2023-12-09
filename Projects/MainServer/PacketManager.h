#pragma once

#include "PacketDefine.h"
#include "type.h"
#include "RouteInjector.h"
#include "ManagerProvider.h"

#include <unordered_map>
#include <deque>
#include <functional>
#include <thread>
#include <mutex>


class UserManager;
class RedisManager;
class User;
class Route;

class PacketManager : public IManagerProvider {

private:
	typedef std::string PACKET_KEY;
	typedef std::function<void(IManagerProvider*, CKPacket::messageHeader*)> PROCESS_RECV_PACKET_FUNCTION;
	typedef std::vector<PROCESS_RECV_PACKET_FUNCTION> ROUTE_CONTAINER;

public:
	PacketManager() = default;
	~PacketManager() = default;

	void Init(const CKServer::int32 maxClient_);

	bool Run();

	void End();

	void ReceivePacketData(const CKServer::int32 clientIndex_, CKPacket::messageHeader* packet);

	void PushSystemPacket(CKPacket::messageHeader* packet_);
		
	std::function<void(CKServer::int32, CKServer::int32, char*)> SendPacketFunc;
	void BroadcastPacket(CKServer::int32 sender, CKServer::int32 size, char* buf, bool includingSender);

	void RegisterCallback(const PACKET_KEY& key, PROCESS_RECV_PACKET_FUNCTION  callback);

public:
	UserManager* GetUserManager() { return mUserManager; }
	PacketManager* GetPacketManager() { return this; }

private:
	void CreateCompent(const CKServer::int32 maxClient_);

	CKPacket::messageHeader* DequePacketData();
	CKPacket::messageHeader* DequeSystemPacketData();

	void ProcessPacket();
	void ProcessRecvPacket(CKPacket::messageHeader* packet);

private:
	std::unordered_map<PACKET_KEY, ROUTE_CONTAINER> mRecvFuntionDictionary;

	UserManager* mUserManager;
	RedisManager* mRedisMgr;
		
	std::function<void(int, char*)> mSendMQDataFunc;


	bool mIsRunProcessThread = false;
	
	std::thread mProcessThread;
	
	std::mutex mLock;
	
	std::deque<CKServer::int32> mInComingPacketUserIndex;

	std::deque<CKPacket::messageHeader*> mSystemPacketQueue;

	RouteInjector mInjector;
};

