#include <VulkanRenderer.h>
#include <CPURenderer.h>
#include <VoxelMap.h>

#include <VCGeneratorSin.h>

#include "VoxelTypes.h"

#define USE_GPU true

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
		ChunkSizeFormat::UINT8, USE_GPU ? ParalsInfoFormat::NO_PARALS : ParalsInfoFormat::NON_CUBIC_UINT8
	};
	VoxelFormat voxFormat = {
		VoxelFullnessFormat::UINT8, VoxelSizeFormat::UINT8,
		VoxelShapeFormat::UINT8, VoxelMaterialFormat::UINT8,
		VoxelOrientationFormat::NO_ORIENTATION, VoxelColorFormat::RGB_THREE_BYTES,
		VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE, USE_GPU ? ParalsInfoFormat::NO_PARALS : ParalsInfoFormat::NON_CUBIC_UINT8,
		&vtc, &vts
	};
	VoxelMapFormat format(chunkFormat, voxFormat);

	VCGeneratorSin generator;
	generator.setGroundType({ vts.getShape(2), vts.getMaterial(1) });

	VoxelMap map(format, generator, 4, 7, 96);
	
	GameState game(&w, map);

	AbstractRenderer* renderer;

	if (USE_GPU) 
	{
		renderer = new VulkanRenderer(w, game);
	}
	else
	{
		renderer = new CPURenderer(w, game);
	}

	renderer->startRender();
	delete renderer;

	return 0;
}