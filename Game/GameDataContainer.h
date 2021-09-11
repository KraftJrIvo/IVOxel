#pragma once

#include <stdint.h>

class GameDataContainer
{
public:
	virtual void setData(uint32_t dataID, void* ptr, uint32_t frameID) = 0;
	virtual bool isGPU() = 0;
};