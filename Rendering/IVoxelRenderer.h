#pragma once

class IVoxelRenderer
{
public:
	virtual void startRender() = 0;
	virtual void stop() = 0;
};