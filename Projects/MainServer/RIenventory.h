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
    // �÷��̾ ����/���� �� DB�� �����͸� ����ȭ ���ִ� ����� �ڵ鷯
    void Connect(IManagerProvider* provider, CKPacket::messageHeader* packet);
    void Disconnect(IManagerProvider* provider, CKPacket::messageHeader* packet);

private:
    /**
    * DB�� �÷��̾� �κ��丮 �����͸�
    * ������ ����ȭ�����ִ� �Լ�
    */
    void UpdateInventoryServerData(IManagerProvider* provider, int userid);

    /**
    * ������ �����ϴ� �÷��̾��� �κ��丮 �����͸�
    * DB�� �������ִ� �Լ�
    */
    void UpdateInventoryDB(IManagerProvider* provider, int userid);

    /** 
    * �÷��̾� �κ��丮 ���̺��� DB�� ���ٸ�, ���������ִ� �Լ�
    */
    void TryCreateInventory(IManagerProvider* provider, int userid);


    /**
    * DB�� ���� ������ �����Ͽ� ���������� ���ŵ�
    * �÷��̾���� �κ��丮 �����͸� DB�� �������ִ� �Լ�
    * ��Ƽ �����忡�� ���� �ֱ⸶�� UpdateInventoryDB �Լ��� ������
    */
    void SynchronizeDB();

private:
    // DB�� ���� �����͸� Write-back ������� ����ȭ ��Ű�� ����
    const int SYNC_DB_INTERVAL_SEC = 3;
    std::thread mSynchroizeThread;
    IManagerProvider* mProvider;
};

