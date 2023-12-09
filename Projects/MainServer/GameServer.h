#pragma once

#include "./ServerNetwork/IOCPServer.h"
#include "PacketManager.h"
#include "ckutil/PacketUtil.h"
#include "PacketDefine.h"

#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>

//TODO redis 연동. hiredis 포함하기

class GameServer : public IOCPServer
{
public:
	GameServer()
	{
		for (int i = 0; i < 10; i++)
		{
			mUserRecvBuf[i] = new char[MAX_SOCK_RECVBUF];
			mUserRecvLastSize[i] = 0;
		}
	}
	virtual ~GameServer() = default;
	

	virtual void OnConnect(const UINT32 clientIndex_) override 
	{
		printf("[OnConnect] 클라이언트: Index(%d)\n", clientIndex_);

		CKPacket::SYSTEM_USER_CONNECT packet;
		auto header = CKPacket::PacketUtil::MakePackedHeader(&packet);
		header->set_userid(clientIndex_);
		m_pPacketManager->PushSystemPacket(header);
	}

	virtual void OnClose(const UINT32 clientIndex_) override 
	{
		printf("[OnClose] 클라이언트: Index(%d)\n", clientIndex_);

		CKPacket::SYSTEM_USER_DISCONNECT packet;
		auto header = CKPacket::PacketUtil::MakePackedHeader(&packet);
		header->set_userid(clientIndex_);
		m_pPacketManager->PushSystemPacket(header);
	}	
	
	virtual void OnReceive(const UINT32 clientIndex_, google::protobuf::Arena& arena, const UINT32 size_, char* pData_) override
	{
		//printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d)\n", clientIndex_, size_);


		UINT32 curPos = 0;
		UINT32& lastSize = mUserRecvLastSize[clientIndex_];
		char* buf1 = mUserRecvBuf[clientIndex_];

		if (lastSize == 0)
		{
			curPos = 0;
		A:
			while (curPos < size_)
			{
				if (curPos + 4 > size_)
				{
					lastSize = size_ - curPos ;
					memcpy(buf1, pData_ + curPos, lastSize);
					return;
				}

				INT32 curSize = CKPacket::PacketUtil::ByteArrToInt32(pData_ + curPos);

				if (curPos + 4 + curSize > size_)
				{
					lastSize = size_ - curPos;
					memcpy(buf1, pData_ + curPos, lastSize);
					return;
				}

				CKPacket::messageHeader* header = nullptr;
				if (CKPacket::PacketUtil::Unpack(nullptr, pData_ + curPos, curSize + 4, &header) == false)
				{
					printf("[OnReceive] 패킷 역직렬화 실패: Index(%d)\n", clientIndex_);
				}
				else
				{
					m_pPacketManager->ReceivePacketData(clientIndex_, header);
				}

				curPos += curSize + 4;
				lastSize = 0;
			}
		}
		else
		{
			curPos = 0;
			INT32 curSize = 0;
			bool c = false;
			if (lastSize >= 4)
			{
				curSize = CKPacket::PacketUtil::ByteArrToInt32(buf1);

				curPos += lastSize;
				memcpy(buf1 + curPos, pData_, curSize + 4 - lastSize);

				curPos = curSize + 4 - lastSize;
			}
			else
			{
				curPos += lastSize;
				memcpy(buf1 + curPos, pData_, 4 - lastSize);
				curSize = CKPacket::PacketUtil::ByteArrToInt32(buf1);
				curPos = 4;

				memcpy(buf1 + curPos, pData_ + (4 - lastSize), curSize);
				curPos = 4 - lastSize + curSize;
				c = true;
			}

			CKPacket::messageHeader* header = nullptr;
			if (CKPacket::PacketUtil::Unpack(nullptr, buf1, curSize + 4, &header) == false)
			{
				printf("[OnReceive] 패킷 역직렬화 실패: Index(%d)\n", clientIndex_);
			}
			else
			{
				m_pPacketManager->ReceivePacketData(clientIndex_, header);
			}



			lastSize = 0;
			goto A;
		}

	}

	void Run(const UINT32 maxClient)
	{
		auto sendPacketFunc = [&](UINT32 clientIndex_, UINT16 packetSize, char* pSendPacket)
		{
			SendMsg(clientIndex_, packetSize, pSendPacket);
		};

		m_pPacketManager = std::make_unique<PacketManager>();
		m_pPacketManager->SendPacketFunc = sendPacketFunc;
		m_pPacketManager->Init(maxClient);		
		m_pPacketManager->Run();
		
		StartServer(maxClient);
	}

	void End()
	{
		m_pPacketManager->End();
		
		DestroyThread();
	}


private:	
	std::unique_ptr<PacketManager> m_pPacketManager;

	char* mUserRecvBuf[10];
	UINT32 mUserRecvLastSize[10];
};