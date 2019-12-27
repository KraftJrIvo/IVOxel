#include "TextFileMap.h"

#include <fstream>
#include <iterator>
#include <sstream>

TextFileMap::TextFileMap(VoxelMapType type) :
	VoxelMap(type)
{
}

void TextFileMap::setFilePath(std::string path)
{
	_filePath = path;
}

int TextFileMap::save()
{
	std::ofstream out(_filePath);

	return 0;
}

int TextFileMap::load()
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

	size_t offset = 0;

	size_t nChunks = std::stoi(txtFileNumbers[offset++]);
	std::vector<int> chunkSize(3);
	for (int i = 0; i < DIMENSIONS; ++i)
		chunkSize[i] = std::stoi(txtFileNumbers[offset++]);

	const size_t voxelCount = chunkSize[X] * chunkSize[Y] * chunkSize[Z];

	std::vector<int> chunkPos(3);
	for (int i = 0; i < nChunks; ++i)
	{
		for (int j = 0; j < DIMENSIONS; ++j)
			chunkPos[j] = std::stoi(txtFileNumbers[offset++]);

		_chunksIds[chunkPos[X]][chunkPos[Y]][chunkPos[Z]] = _chunks.size();
		_chunks.push_back(VoxelChunk(chunkSize, _type));
		VoxelChunk& chunk = _chunks[_chunks.size() - 1];

		chunk.vTypes.reserve(voxelCount);

		// parsing voxels
		auto it = txtFileNumbers.begin() + offset;
		for (int j = 0; j < voxelCount; ++j)
		{
			auto& voxTxt = *it++;
			unsigned char vox = (voxTxt[0] == '-') ? 255 : std::stoi(voxTxt);

			chunk.vTypes.push_back(vox);
		}

		// reversing Y for convenience
		auto temp = chunk.vTypes;
		for (int j = 0; j < chunkSize[Z]; ++j)
			for (int k = 0; k < chunkSize[Y]; ++k)
				for (int l = 0; l < chunkSize[X]; ++l)
					chunk.vTypes[chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * k + l] =
					temp[chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * (chunkSize[Y] - k - 1) + l];

		// parsing voxel colors
		std::vector<std::vector<unsigned char>> colors(RGBA);
		for (int j = 0; j < RGBA; ++j)
		{
			colors[j].reserve(voxelCount);
			for (int k = 0; k < voxelCount; ++k)
			{
				unsigned char vox = std::stoi(*it++);

				colors[j].push_back(vox);
			}
		}

		// again, reversing Y for convenience (also copying to chunk)
		const size_t& sz = colors[R].size();
		chunk.vColors.reserve(sz * RGBA);
		for (int j = 0; j < chunkSize[Z]; ++j)
			for (int k = 0; k < chunkSize[Y]; ++k)
				for (int l = 0; l < chunkSize[X]; ++l)
					for (int m = 0; m < RGBA; ++m)
						chunk.vColors.push_back(colors[m][chunkSize[X] * chunkSize[Y] * j + chunkSize[X] * (chunkSize[Y] - k - 1) + l]);

		// parsing lights
		unsigned char nLights = std::stoi(*it++);
		for (int j = 0; j < nLights; ++j)
		{
			Light light;
			for (int k = 0; k < RGBA; ++k)
				light.rgba[k] = std::stoi(*it++);
			for (int k = 0; k < DIMENSIONS; ++k)
				light.position[k] = std::stof(*it++);
			_lights.push_back(light);
		}
	}

	return 0;
}
