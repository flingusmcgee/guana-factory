#pragma once
#include <vector>
#include <string>
#include "include/raylib.h"
#include "ChunkPath.h"
#include "WorldGenerator.h"
#include "Quad.h"


typedef unsigned char BlockId;


class Chunk {
public:
static constexpr int SIZE = 16;


bool hasModel = false;
Model model{};
std::vector<Quad> quads;


Chunk();
~Chunk();

void Init(const ChunkCoord& c);
void InitWithPath(const ChunkPath& p);


void SetPath(const ChunkPath& p);
const ChunkPath& GetPath() const;


const ChunkCoord& GetCoord() const;


std::string GetIdentifier() const;


BlockId Get(int x, int y, int z) const;


void Set(int x, int y, int z, BlockId id);


bool Save(const std::string&) const;
bool Load(const std::string&);


private:
ChunkCoord coord{};
ChunkPath path;
std::vector<BlockId> blocks = std::vector<BlockId>(SIZE*SIZE*SIZE, 0);
};
