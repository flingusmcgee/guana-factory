#include "ChunkRegistry.h"
#include "Log.h"

bool ChunkRegistry::Add(const std::shared_ptr<Chunk>& chunk) {
    if (!chunk) {
        Log::Error("ChunkRegistry::Add called with nullptr");
        return false;
    }

    uint32_t hash = chunk->GetPath().Hash();
    
    if (registry.find(hash) != registry.end()) {
        Log::Warning("ChunkRegistry::Add - Path " + chunk->GetIdentifier() + " already registered");
        return false;
    }

    registry[hash] = chunk;
    Log::Info("ChunkRegistry::Add - Registered chunk at " + chunk->GetIdentifier() + " (hash: " + std::to_string(hash) + ")");
    return true;
}

std::shared_ptr<Chunk> ChunkRegistry::Remove(const ChunkPath& path) {
    uint32_t hash = path.Hash();
    auto it = registry.find(hash);
    
    if (it == registry.end()) {
        return nullptr;
    }

    auto chunk = it->second;
    registry.erase(it);
    Log::Info("ChunkRegistry::Remove - Unregistered chunk at " + chunk->GetIdentifier());
    return chunk;
}

const std::shared_ptr<Chunk> ChunkRegistry::Get(const ChunkPath& path) const {
    uint32_t hash = path.Hash();
    auto it = registry.find(hash);
    
    if (it == registry.end()) {
        return nullptr;
    }

    return it->second;
}

bool ChunkRegistry::Contains(const ChunkPath& path) const {
    uint32_t hash = path.Hash();
    return registry.find(hash) != registry.end();
}

std::vector<std::shared_ptr<Chunk>> ChunkRegistry::GetAll() {
    std::vector<std::shared_ptr<Chunk>> chunks;
    for (auto& pair : registry) {
        chunks.push_back(pair.second);
    }
    return chunks;
}

void ChunkRegistry::Clear() {
    int count = registry.size();
    registry.clear();
    Log::Info("ChunkRegistry::Clear - Cleared " + std::to_string(count) + " chunks");
}
