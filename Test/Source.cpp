//#include <GPURenderer.h>
#include <CPURenderer.h>
#include <VoxelMap.h>

#include <VCGeneratorSin.h>

#include "VoxelTypes.h"

int main()
{
	Window w(200, 200, L"test");

	VoxelTypeStorer vts;
	vts.addShape(1, std::make_shared<ShapeCube>());
	vts.addShape(2, std::make_shared<ShapeSphere>());
	vts.addMaterial(1, std::make_shared<MaterialDefault>());
	TestVoxelTypeCoder vtc(vts);

	VoxelChunkFormat chunkFormat = {
		ChunkFullnessFormat::UINT8, ChunkOffsetFormat::UINT32,
		ChunkSizeFormat::UINT8, ParalsInfoFormat::NON_CUBIC_FLOAT32
	};
	VoxelFormat voxFormat = {
		VoxelFullnessFormat::UINT8, VoxelSizeFormat::UINT8, 
		VoxelShapeFormat::UINT8, VoxelMaterialFormat::UINT8, 
		VoxelOrientationFormat::NO_ORIENTATION, VoxelColorFormat::RGB256,
		VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE, ParalsInfoFormat::NON_CUBIC_UINT8,
		&vtc, &vts
	};
	VoxelMapFormat format(chunkFormat, voxFormat);

	VCGeneratorSin generator;
	generator.setGroundType({ vts.getShape(2), vts.getMaterial(1) });

	VoxelMap map(format, generator, 4, 2);
	
	GameState game;
	game.init(&w, 1.0f);
	game.setMap(&map);

	CPURenderer renderer(w, game, map.getLoadRadius());

	renderer.startRender();

	return 0;
}