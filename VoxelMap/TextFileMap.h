#pragma once

#include "VoxelMap.h"

class TextFileMap : public VoxelMap
{
public:
	TextFileMap();
	virtual int save();
	virtual int load();

private:
	virtual int _ivoxelize();
};