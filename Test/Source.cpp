#include <TextFileMap.h>
#include <Camera.h>
#include <CPURenderer.h>

int main()
{
	CPURenderer renderer;
	
	TextFileMap map(THREE_BYTES_RGB256_ALPHA_AND_TYPE);
	map.setFilePath("test_map.txt");
	map.load();
	
	Camera cam(90, {640, 480});
	cam.move({ 0, 0, -0.5f });

	renderer.render(map, cam);

	return 0;
}