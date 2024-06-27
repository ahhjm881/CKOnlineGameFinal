#pragma once
#include "Route.h"
#include <random>

class RShop :
    public Route
{
public:
    RShop();

    virtual void RegisterPacket(class PacketManager* packetManager) override;

private:
    /**
    * 아이템 구매/판매 요청에 대한 핸들러
    */
    void Buy(IManagerProvider* provider, CKPacket::messageHeader* packet);
    void Sell(IManagerProvider* provider, CKPacket::messageHeader* packet);

    /**
    * 클라이언트에서 상점 아이템 갱신에 대한 핸들러
    */
    void RequireItemList(IManagerProvider* provider, CKPacket::messageHeader* packet);

private:

    /**
    * 일정 주기마다 상점 아이템 목록을 초기화해주는 함수.
    * 멀티 스레드에서 동작함
    */
    void ResetItemUpdate();

    /**
    * DB에서 랜덤하게 아이템들을 조회하여
    * 상점 아이템을 초기화하는 함수.
    */
    void ResetItem(std::mt19937&, std::uniform_int_distribution<>&);

    /**
    * 클라이언트의 인벤토리를 갱신해주는 함수
    */
    void UpdateClientInventory(IManagerProvider* provider, std::string userName);

    /**
    * 현재 상점 아이템 목록을 특정 유저 or 전체 유저에게
    * 보내어 클라이언트의 상점 아이템 목록을 갱신시킴
    */
    void SendCurrentShopItemList(int userid, bool useBroadcast);
    

private:
    using ItemType = std::tuple<int, std::string, std::string, int>;
    using ItemListType = std::vector<ItemType>;

    std::thread mResetThread;
    ItemListType mItemList;
    std::mutex mResetMutex;

    class PacketManager* mPacketManager;

    const int SAMPLE_ITEM_COUNT = 100000;
    const int RESET_INTERVAL_SEC = 5;
    const int ITEM_LIST_MAX_COUNT = 30;
    const int DEFAULT_ITEM_COUNT = 30;
};

