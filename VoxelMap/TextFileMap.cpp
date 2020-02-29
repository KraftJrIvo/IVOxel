#include "TextFileMap.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <algorithm>

TextFileMap::TextFileMap(const VoxelMapType& type) :
	VoxelMap(type)
{
}

void TextFileMap::setFilePath(std::string path)
{
	_filePath = path;
}

void TextFileMap::save()
{
	std::ofstream out(_filePath);

	//TODO?
}

void TextFileMap::load()
{
	std::ifstream in(_filePath);

	std::stringstream str;

	// excluding comments and empty lines
	std::string line;
	while (!in.eof())
	{
		std::getline(in, line);
		if (line.size() && line.at(0) != '#') str << line << " ";
	}

	std::vector<std::string> txtFileNumbers((std::istream_iterator<utils::WordDelimitedBySpace>(str)),
		std::istream_iterator<utils::WordDelimitedBySpace>());

	uint32_t offset = 0;

	auto it = txtFileNumbers.begin();

	uint32_t nChunks = std::stoi(*it++);
	std::vector<uint32_t> chunkSize(DIMENSIONS);
	std::vector<int32_t> chunkPos(DIMENSIONS);

	for (uint32_t i = 0; i < nChunks; ++i)
	{
		for (uint8_t j = 0; j < DIMENSIONS; ++j)
			chunkSize[j] = std::stoi(*it++);

		const uint32_t voxelCount = chunkSize[X] * chunkSize[Y] * chunkSize[Z];

		for (uint8_t j = 0; j < DIMENSIONS; ++j)
			chunkPos[j] = std::stoi(*it++);

		VoxelChunk& chunk = _addChunk(chunkPos, chunkSize);

		chunk.vTypes.reserve(voxelCount);

		// parsing voxels		
		for (uint32_t j = 0; j < voxelCount; ++j)
		{
			auto& voxTxt = *it++;
			uint8_t vox = (voxTxt[0] == '-') ? 255 : std::stoi(voxTxt);

			auto formattedType = _type.formatType(vox);
			chunk.vTypes = utils::joinVectors(chunk.vTypes, formattedType);
		}

		// reversing Y for convenience
		auto temp = chunk.vTypes;
		for (uint32_t j = 0; j < chunkSize[Z]; ++j)
			for (uint32_t k = 0; k < chunkSize[Y]; ++k)
				for (uint32_t l = 0; l < chunkSize[X]; ++l)
					for (uint8_t m = 0; m < _type.sizeInBytesType; ++m)
						chunk.vTypes[_type.sizeInBytesType * (chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * k + l) + m] =
						temp[_type.sizeInBytesType * (chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * (chunkSize[Y] - k - 1) + l) + m];

		// parsing voxel colors
		std::vector<std::vector<uint8_t>> colors(RGBA);
		for (uint8_t j = 0; j < RGBA; ++j)
		{
			colors[j].reserve(voxelCount);
			for (uint32_t k = 0; k < voxelCount; ++k)
			{
				uint8_t vox = std::stoi(*it++);

				colors[j].push_back(vox);
			}
		}

		// again, reversing Y for convenience (also copying to chunk)
		const uint32_t& sz = colors[R].size();
		chunk.vColors.reserve(sz * RGBA);
		for (uint32_t j = 0; j < chunkSize[Z]; ++j)
			for (uint32_t k = 0; k < chunkSize[Y]; ++k)
				for (uint32_t l = 0; l < chunkSize[X]; ++l)
				{
					uint32_t offset = chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * (chunkSize[Y] - k - 1) + l;
					uint8_t r = colors[R][offset];
					uint8_t g = colors[G][offset];
					uint8_t b = colors[B][offset];
					uint8_t a = colors[A][offset];
					auto formattedColor = _type.formatColor(r, g, b, a);
					chunk.vColors = utils::joinVectors(chunk.vColors, formattedColor);
				}

		// parsing lights
		uint8_t nLights = std::stoi(*it++);
		_lightsByChunks.push_back(std::vector<Light>());
		_lightsByChunks.back().reserve(nLights);
		for (uint32_t j = 0; j < nLights; ++j)
		{
			Light light;
			for (uint8_t k = 0; k < RGBA; ++k)
				light.rgba[k] = std::stoi(*it++);
			for (uint8_t k = 0; k < DIMENSIONS; ++k)
				light.position[k] = std::stof(*it++);
			_lightsByChunks[i].push_back(light);
		}
	}
}
