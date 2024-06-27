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
    * ������ ����/�Ǹ� ��û�� ���� �ڵ鷯
    */
    void Buy(IManagerProvider* provider, CKPacket::messageHeader* packet);
    void Sell(IManagerProvider* provider, CKPacket::messageHeader* packet);

    /**
    * Ŭ���̾�Ʈ���� ���� ������ ���ſ� ���� �ڵ鷯
    */
    void RequireItemList(IManagerProvider* provider, CKPacket::messageHeader* packet);

private:

    /**
    * ���� �ֱ⸶�� ���� ������ ����� �ʱ�ȭ���ִ� �Լ�.
    * ��Ƽ �����忡�� ������
    */
    void ResetItemUpdate();

    /**
    * DB���� �����ϰ� �����۵��� ��ȸ�Ͽ�
    * ���� �������� �ʱ�ȭ�ϴ� �Լ�.
    */
    void ResetItem(std::mt19937&, std::uniform_int_distribution<>&);

    /**
    * Ŭ���̾�Ʈ�� �κ��丮�� �������ִ� �Լ�
    */
    void UpdateClientInventory(IManagerProvider* provider, std::string userName);

    /**
    * ���� ���� ������ ����� Ư�� ���� or ��ü ��������
    * ������ Ŭ���̾�Ʈ�� ���� ������ ����� ���Ž�Ŵ
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

