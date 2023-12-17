#pragma once

#include "Position.h"
#include "IPlayer.h"

#include <memory>
#include <forward_list>

using ObserverPtr = std::weak_ptr<struct IGameObserver>;
using ObserverList = std::forward_list<ObserverPtr>;

struct IGameObserver
{
	virtual void OnColumnPlaced(Position& position, IPlayer* player) = 0;
	virtual void OnBridgePlaced(Position& lhs, Position& rhs, IPlayer* player) = 0;
	virtual void OnBridgeRemoved(Position& lhs, Position& rhs, IPlayer* player) = 0;

	virtual ~IGameObserver() = default;
};
