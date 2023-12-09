#include <iostream>
#include <fstream>
#include <sstream>

#include "PacketDefine.h"
#include "PacketUtil.h"

using namespace std;
using namespace CKPacket;

int Test()
{
    itemB item;
    item.set_name("aaa");
    item.set_id(132);


    char* buf = new char[8192];
    char* packedBuf = new char[8192];
    size_t len;
    size_t maxLen = 8192;
    size_t packedLen;
    size_t packedmaxLen = 8192;
    messageHeader* header1 = new messageHeader;
    messageHeader* header2 = new messageHeader;

    google::protobuf::Arena arena;
    
    if (PacketUtil::Serialize(&item, buf, maxLen, &len) == false)
    {
        cout << "seralize fail" << endl;
        return 1;
    }
    if (PacketUtil::Pack(header1, buf, len, item.GetDescriptor()->full_name().data(), packedBuf, &packedLen, packedmaxLen) == false)
    {
        cout << "pack fail" << endl;
        return 1;
    }
    buf = nullptr;

    itemB* deserializedItem;
    
    if (PacketUtil::Unpack(nullptr, packedBuf, packedLen, &header2) == false)
    {
        cout << "unpack fail" << endl;
        return 1;
    }

    if (PacketUtil::Deserialize(nullptr, header2->arr().data(), header2->arrsize(), header2->type().data(), &deserializedItem) == false)
    {
        cout << "deseralize fail" << endl;
        return 1;
    }

    cout << deserializedItem->name() << endl;
    cout << deserializedItem->id() << endl;

    delete deserializedItem;

    return 0;
}