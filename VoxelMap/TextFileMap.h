#pragma once

#include "VoxelMap.h"

#include <string>

class TextFileMap : public VoxelMap
{
public:
	TextFileMap(const VoxelMapType& type);

	void setFilePath(std::string path);

	virtual void save();
	virtual void load();

private:
	std::string _filePath;

};