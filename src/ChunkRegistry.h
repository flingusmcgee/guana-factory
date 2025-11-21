#pragma once
#include <unordered_map>
#include <memory>
#include <vector>
#include "Chunk.h"
#include "ChunkPath.h"


class ChunkRegistry {
public:
bool Add(const std::shared_ptr<Chunk>& chunk);


std::shared_ptr<Chunk> ChunkRegistry::Remove(const ChunkPath& path);
const std::shared_ptr<Chunk> ChunkRegistry::Get(const ChunkPath& path) const;


bool ChunkRegistry::Contains(const ChunkPath& path) const;


std::vector<std::shared_ptr<Chunk>> GetAll() {
std::vector<std::shared_ptr<Chunk>> out;
out.reserve(chunks.size());
for (auto& [k, v] : chunks) out.push_back(v);
return out;
}


void Clear() {
chunks.clear();
}


private:
std::unordered_map<uint32_t, std::shared_ptr<Chunk>> chunks;
};
