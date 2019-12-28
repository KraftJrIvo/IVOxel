#include "TextFileMap.h"

#include <fstream>
#include <iterator>
#include <sstream>

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

	std::vector<std::string> txtFileNumbers((std::istream_iterator<WordDelimitedBySpace>(str)),
		std::istream_iterator<WordDelimitedBySpace>());

	uint32_t offset = 0;

	uint32_t nChunks = std::stoi(txtFileNumbers[offset++]);
	std::vector<uint32_t> chunkSize(3);
	for (uint8_t i = 0; i < DIMENSIONS; ++i)
		chunkSize[i] = std::stoi(txtFileNumbers[offset++]);

	const uint32_t voxelCount = chunkSize[X] * chunkSize[Y] * chunkSize[Z];

	std::vector<int32_t> chunkPos(3);
	for (uint32_t i = 0; i < nChunks; ++i)
	{
		for (uint8_t j = 0; j < DIMENSIONS; ++j)
			chunkPos[j] = std::stoi(txtFileNumbers[offset++]);

		_chunksIds[chunkPos[X]][chunkPos[Y]][chunkPos[Z]] = _chunks.size();
		_chunks.push_back(VoxelChunk(chunkSize, _type));
		VoxelChunk& chunk = _chunks[_chunks.size() - 1];

		chunk.vTypes.reserve(voxelCount);

		// parsing voxels
		auto it = txtFileNumbers.begin() + offset;
		for (uint32_t j = 0; j < voxelCount; ++j)
		{
			auto& voxTxt = *it++;
			uint8_t vox = (voxTxt[0] == '-') ? 255 : std::stoi(voxTxt);

			chunk.vTypes.push_back(vox);
		}

		// reversing Y for convenience
		auto temp = chunk.vTypes;
		for (uint32_t j = 0; j < chunkSize[Z]; ++j)
			for (uint32_t k = 0; k < chunkSize[Y]; ++k)
				for (uint32_t l = 0; l < chunkSize[X]; ++l)
					chunk.vTypes[chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * k + l] =
					temp[chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * (chunkSize[Y] - k - 1) + l];

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
					for (uint8_t m = 0; m < RGBA; ++m)
						chunk.vColors.push_back(colors[m][chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * (chunkSize[Y] - k - 1) + l]);

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
