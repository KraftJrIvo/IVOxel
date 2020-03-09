#include <CPURenderer.h>
#include <GPURenderer.h>
#include <TextFileMap.h>
#include <Camera.h>

int main()
{
	GPURenderer renderer;
	
	VoxelMapType mapType(VoxelTypeFormat::UINT8, VoxelColorFormat::RGB256, VoxelNeighbourInfoFormat::NO_NEIGHBOUR_INFO);

	TextFileMap map(mapType);
	map.setFilePath("test_map.txt");
	map.load();

	map.buildPyramid();
	
	Camera cam(90, {640, 480});
	cam.rotate({ 0.5f,0,0 });
	cam.move({ 0.6f, 1.6f, -1.2f });

	renderer.render(map, cam);
	//renderer.renderVideo(map, cam);

	return 0;
}