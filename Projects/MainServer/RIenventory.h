#pragma once
#include "Route.h"
#include <jdbc/cppconn/resultset.h>
#include <string>

class RIenventory :
    public Route
{

private:
    using ItemType = std::tuple<int, std::string, std::string, int>;
    using ItemListType = std::vector<ItemType>;

public:
    RIenventory();

    virtual void RegisterPacket(class PacketManager* packetManager) override;

private:
    // 플레이어가 접속/퇴장 시 DB의 데이터를 동기화 해주는 기능의 핸들러
    void Connect(IManagerProvider* provider, CKPacket::messageHeader* packet);
    void Disconnect(IManagerProvider* provider, CKPacket::messageHeader* packet);

private:
    /**
    * DB의 플레이어 인벤토리 데이터를
    * 서버에 동기화시켜주는 함수
    */
    void UpdateInventoryServerData(IManagerProvider* provider, int userid);

    /**
    * 서버에 존재하는 플레이어의 인벤토리 데이터를
    * DB에 저장해주는 함수
    */
    void UpdateInventoryDB(IManagerProvider* provider, int userid);

    /** 
    * 플레이어 인벤토리 테이블이 DB에 없다면, 생성시켜주는 함수
    */
    void TryCreateInventory(IManagerProvider* provider, int userid);


    /**
    * DB와 현재 서버에 접속하여 마지막으로 갱신된
    * 플레이어들의 인벤토리 데이터를 DB에 저장해주는 함수
    * 멀티 스레드에서 일정 주기마다 UpdateInventoryDB 함수를 콜해줌
    */
    void SynchronizeDB();

private:
    // DB와 서버 데이터를 Write-back 방식으로 동기화 시키는 간격
    const int SYNC_DB_INTERVAL_SEC = 3;
    std::thread mSynchroizeThread;
    IManagerProvider* mProvider;
};

