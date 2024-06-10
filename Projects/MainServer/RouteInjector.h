#pragma once

#include "Route.h"

#include "RConnection.h"
#include "RPing.h"
#include "RReplicationPlayer.h"
#include "RItemGiver.h"

#include <vector>

class RouteInjector
{
public:
	template<class T>
	RouteInjector& AddRoute()
	{
		mRoutes.push_back(new T());
		return *this;
	}

	void RouteInit(class PacketManager& manager)
	{
		AddRoute<RConnection>();
		AddRoute<RPing>();
		AddRoute<RReplicationPlayer>();
		AddRoute<RItemGiver>();

		for (auto i : mRoutes)
		{
			i->RegisterPacket(&manager);
		}
	}

private:
	std::vector<Route*> mRoutes;
};