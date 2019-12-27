#include "Light.h"

Light::Light() :
	position({0,0,0}),
	rgba({0,0,0,0})
{
}

Light::Light(const std::vector<float>& _position, const std::vector<unsigned char>& _rgba) :
	position(_position),
	rgba(_rgba)
{
}
