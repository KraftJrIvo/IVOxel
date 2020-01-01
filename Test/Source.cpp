#include <CPURenderer.h>
#include <TextFileMap.h>
#include <Camera.h>

int main()
{
	CPURenderer renderer;
	
	VoxelMapType mapType(VoxelTypeFormat::UINT8, VoxelColorFormat::RGB256, VoxelNeighbourInfoFormat::NO_NEIGHBOUR_INFO);

	TextFileMap map(mapType);
	map.setFilePath("test_map.txt");
	map.load();

	map.buildPyramid();
	
	Camera cam(90, {640, 480});
	cam.move({ 0, 0, -0.5f });

	renderer.render(map, cam);

	return 0;
}