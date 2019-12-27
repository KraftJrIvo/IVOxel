#pragma once

#include "VoxelMap.h"

#include <string>

class TextFileMap : public VoxelMap
{
public:
	TextFileMap(VoxelMapType type);

	void setFilePath(std::string path);

	virtual int save();
	virtual int load();

private:
	std::string _filePath;

};