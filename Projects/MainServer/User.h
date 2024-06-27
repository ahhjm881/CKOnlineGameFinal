#pragma once

#include "PacketDefine.h"
#include "type.h"

#include <string>
#include <queue>

class User
{
public:
	using ItemType = std::tuple<int, std::string, std::string, int>;
	using ItemListType = std::vector<ItemType>;

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

	void AddItemCount(ItemType item)
	{
		std::lock_guard<std::mutex> guard(mItemListMutex);

		for (auto i = mItemList.begin(); i != mItemList.end(); i++)
		{
			if (std::get<0>(*i) == std::get<0>(item))
			{
				int c = std::get<3>(*i);
				c += std::get<3>(item);

				if (c <= 0)
				{
					mItemList.erase(i);
				}
				else
				{
					ItemType tuple = *i;
					std::get<3>(tuple) = c;
					*i = tuple;
				}
				return;
			}
		}

		mItemList.push_back(item);
	}

	bool TryGetItem(int itemid, ItemType* item)
	{
		std::lock_guard<std::mutex> guard(mItemListMutex);

		for (auto& i : mItemList)
		{
			if (std::get<0>(i) == itemid)
			{
				*item = i;
				return true;
			}
		}

		return false;
	}

	void GetItemAll(ItemListType& list)
	{
		std::lock_guard<std::mutex> guard(mItemListMutex);

		for (auto& i : mItemList)
		{
			list.push_back(i);
		}
	}

	void ClearItem()
	{
		std::lock_guard<std::mutex> guard(mItemListMutex);
		mItemList.clear();
	}

	void SetName(std::string name)
	{
		std::lock_guard<std::mutex> guard(mNameMutex);
		mName = name;
	}

	std::string GetName()
	{
		std::lock_guard<std::mutex> guard(mNameMutex);
		return mName;
	}

private:
	CKServer::int32 mIndex = -1;
	std::string mName;

	bool mIsConfirm = false;
	std::string mAuthToken;
	std::queue<CKPacket::messageHeader*> mPacketQueue;

	std::mutex mItemListMutex;
	ItemListType mItemList;

	std::mutex mNameMutex;
};

