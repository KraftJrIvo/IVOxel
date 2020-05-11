#pragma once

struct LightingShaderInfo {
    int32_t nLights;
    float absCoords[16];
    int32_t colors[4 * 16];
};