#pragma once

#include "IColumn.h"
#include "Position.h"

struct _declspec(dllexport) IBoard
{
	virtual const uint16_t GetSize() const = 0;

	virtual const IColumn* GetElement(Position pos) const = 0;
	virtual const IColumn* GetElement(int row, int column) const = 0;

	virtual const IColumn* operator[](Position pos) const = 0;

	virtual ~IBoard() = default;
};