#include "HeightmapGenerator.h"
#include <cmath>

HeightmapGenerator::HeightmapGenerator() {}

// Simple pseudo-noise function: combines sines for a deterministic heightfield.
static float pseudoNoise(int x, int z, float scale) {
    float fx = static_cast<float>(x) * scale;
    float fz = static_cast<float>(z) * scale;
    return (std::sin(fx * 0.12f) + std::sin(fz * 0.08f) * 0.8f + std::sin((fx+fz) * 0.05f) * 0.5f) * 0.5f;
}

float HeightmapGenerator::SampleHeight(int wx, int wz) const {
    float n = pseudoNoise(wx, wz, 1.0f);
    float base = 4.0f; // base elevation
    float amp = 6.0f;  // amplitude
    return base + n * amp; // final height
}

void HeightmapGenerator::GenerateChunk(const ChunkCoord& coord, BlockId* outBlocks, int chunkSize) {
    // Clear to air
    int volume = chunkSize * chunkSize * chunkSize;
    for (int i = 0; i < volume; ++i) outBlocks[i] = 0;

    // For every x,z column in the chunk, sample world-space height and fill blocks below with dirt (id=1)
    for (int lx = 0; lx < chunkSize; ++lx) {
        for (int lz = 0; lz < chunkSize; ++lz) {
            int wx = coord.x * chunkSize + lx;
            int wz = coord.z * chunkSize + lz;
            float h = SampleHeight(wx, wz);
            int ih = static_cast<int>(std::floor(h));
            for (int y = 0; y < chunkSize; ++y) {
                int wy = coord.y * chunkSize + y;
                if (wy <= ih) {
                    int index = lx + lz * chunkSize + y * chunkSize * chunkSize;
                    outBlocks[index] = 1; // dirt
                }
            }
        }
    }
}
