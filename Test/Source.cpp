#include <VulkanRenderer.h>
#include <CPURenderer.h>
#include <VoxelMap.h>

#include <VCGeneratorSin.h>

#include "VoxelTypes.h"

#define USE_GPU true

int main()
{
	Window w(200, 200, L"test", USE_GPU ? 2.0 : 1.0);

	VoxelTypeStorer vts;
	vts.addShape(1, std::make_shared<ShapeCube>());
	vts.addShape(2, std::make_shared<ShapeSphere>());
	vts.addMaterial(1, std::make_shared<MaterialDefault>());
	TestVoxelTypeCoder vtc(vts);

	VoxelChunkFormat chunkFormat = {
		USE_GPU ? ChunkFullnessFormat::UINT32 : ChunkFullnessFormat::UINT8,
		ChunkOffsetFormat::UINT32,
		USE_GPU ? ChunkSizeFormat::UINT32 : ChunkSizeFormat::UINT8,
		USE_GPU ? ParalsInfoFormat::NO_PARALS : ParalsInfoFormat::NON_CUBIC_UINT8
	};
	VoxelFormat voxFormat = {
		USE_GPU ? VoxelFullnessFormat::UINT32 : VoxelFullnessFormat::UINT8,
		USE_GPU ? VoxelSizeFormat::UINT32 : VoxelSizeFormat::UINT8,
		USE_GPU ? VoxelShapeFormat::UINT32 : VoxelShapeFormat::UINT8, 
		USE_GPU ? VoxelMaterialFormat::UINT32 : VoxelMaterialFormat::UINT8,
		VoxelOrientationFormat::NO_ORIENTATION, 
		USE_GPU ? VoxelColorFormat::RGBA_FOUR_BYTES : VoxelColorFormat::RGB_THREE_BYTES,
		USE_GPU ? VoxelNeighbourInfoFormat::TWENTY_SIX_DIRS_FOUR_BYTES : VoxelNeighbourInfoFormat::SIX_DIRS_ONE_BYTE,
		USE_GPU ? ParalsInfoFormat::NO_PARALS : ParalsInfoFormat::NON_CUBIC_UINT8,
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