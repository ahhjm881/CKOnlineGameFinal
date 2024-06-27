#pragma once
#include <unordered_set>

#include "ErrorCode.h"
#include "User.h"
#include "type.h"

class UserManager
{
public:
	UserManager() = default;
	~UserManager() = default;

	void Init(const CKServer::int32 maxUserCount_)
	{
		mMaxUserCnt = maxUserCount_;
		mUserObjPool = std::vector<User*>(mMaxUserCnt);

		for (auto i = 0; i < mMaxUserCnt; i++)
		{
			mUserObjPool[i] = new User();
			mUserObjPool[i]->Init(i);
		}
	}

	ERROR_CODE AddUser(char* userID_, int clientIndex_)
	{
		std::lock_guard<std::mutex> lock(mLock);

		auto user_idx = clientIndex_;

		mUserObjPool[user_idx]->Clear();
		mUserIDDictionary.insert(clientIndex_);

		IncreaseUserCnt();

		return ERROR_CODE::NONE;
	}

	void DeleteUser(User* user_)
	{
		std::lock_guard<std::mutex> lock(mLock);
		mUserIDDictionary.erase(user_->GetNetConnIdx());
		user_->Clear();


		DecreaseUserCnt();
	}

	User* GetUserByConnIdx(CKServer::int32 clientIndex_)
	{
		std::lock_guard<std::mutex> lock(mLock);
		return mUserObjPool[clientIndex_];
	}


	User* GetUserByName(std::string name)
	{
		std::lock_guard<std::mutex> lock(mLock);

		for (int i = 0; i < mMaxUserCnt; i++)
		{
			if (mUserObjPool[i]->GetName() == name)
			{
				return mUserObjPool[i];
			}
		}

		return nullptr;
	}

	void GetAllUser(std::vector<User*>& v)
	{
		std::lock_guard<std::mutex> lock(mLock);
		for (auto i : mUserIDDictionary)
			v.push_back(mUserObjPool[i]);
	}

	CKServer::int32 GetCurrentUserCnt() 
	{
		std::lock_guard<std::mutex> lock(mLock); 
		return mCurrentUserCnt; 
	}

	CKServer::int32 GetMaxUserCnt() 
	{
		std::lock_guard<std::mutex> lock(mLock);
		return mMaxUserCnt; 
	}

private:

	void IncreaseUserCnt() { mCurrentUserCnt++; }

	void DecreaseUserCnt()
	{
		if (mCurrentUserCnt > 0)
		{
			mCurrentUserCnt--;
		}
	}

private:
	CKServer::int32 mMaxUserCnt = 0;
	CKServer::int32 mCurrentUserCnt = 0;

	std::vector<User*> mUserObjPool; //vector·Î
	std::unordered_set<int> mUserIDDictionary;

	std::mutex mLock;
};
