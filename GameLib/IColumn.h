#pragma once

#include "IPlayer.h"

struct _declspec(dllexport) IColumn
{
	virtual IPlayer* GetPlayer() const noexcept = 0;

	virtual ~IColumn() = default;
};