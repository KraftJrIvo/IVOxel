#include "Light.h"

Light::Light(LightType lt, const std::vector<uint8_t> _rgba, const std::vector<float> _position) :
	type(lt),
	rgba(_rgba),
	position(_position)
{ }
