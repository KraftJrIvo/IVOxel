#pragma once

#include <stdint.h>

class GameDataContainer
{
public:
	virtual void setData(uint32_t frameID, uint32_t dataID, void* ptr) = 0;
};