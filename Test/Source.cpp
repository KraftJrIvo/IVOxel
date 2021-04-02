//#include <GPURenderer.h>
#include <CPURenderer.h>
#include <VoxelMap.h>
//#include <Window.h>

int main()
{
	Window w(512, 512, L"test");
	GameState game;
	game.init(&w, 1.0f);
	CPURenderer renderer(game);

	VoxelFormat voxFormat;
	VoxelChunkFormat voxChunkFormat;
	VoxelMapFormat format;
	VoxelMap map(const VoxelMapFormat & format, VoxelChunkGenerator & generator, uint32_t loadRadius = 7);
	//GPURenderer renderer;
	////CPURenderer renderer;
	//
	//VoxelMapFormat mapFormat(VoxelTypeFormat::UINT8, VoxelColorFormat::RGB256, VoxelNeighbourInfoFormat::NO_NEIGHBOUR_INFO);

	//TextFileMap map(mapFormat);
	//map.setFilePath("test_map.txt");
	//map.load();

	//map.buildPyramid();
	//
	//Camera cam(90, {640, 480});
	//cam.rotate({ 0.5f,0,0 });
	//cam.move({ 0.6f, 1.6f, -1.2f });

	//renderer.render(map, cam);
	////renderer.renderVideo(map, cam);

	//return 0;
}