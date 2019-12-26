#include <TextFileMap.h>
#include <CPURenderer.h>

int main()
{
	CPURenderer renderer;
	
	TextFileMap map;
	map.load();
	
	renderer.render(map);

	return 0;
}