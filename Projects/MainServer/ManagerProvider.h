#pragma once


__interface IManagerProvider
{
	class UserManager* GetUserManager();
	class PacketManager* GetPacketManager();
};