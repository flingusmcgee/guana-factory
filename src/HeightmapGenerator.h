#pragma once
#include "WorldGenerator.h"

class HeightmapGenerator : public WorldGenerator {
public:
    HeightmapGenerator();
    virtual void GenerateChunk(const ChunkCoord& coord, BlockId* outBlocks, int chunkSize) override;
private:
    float SampleHeight(int wx, int wz) const;
};
