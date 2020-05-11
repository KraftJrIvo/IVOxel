#pragma once

struct MapShaderInfo {
    int32_t chunkOffsets[27];
    char chunkData[sizeof(int32_t) * 1024];
};