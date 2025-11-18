#include "Chunk.h"
#include "Log.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

Chunk::Chunk() : blocks(SIZE * SIZE * SIZE, 0) {
    coord = {0,0,0};
    hasModel = false;
    model = {};
    quads.clear();
}

Chunk::~Chunk() {
    if (hasModel) {
        UnloadModel(model);
        hasModel = false;
    }
    quads.clear();
}

void Chunk::Init(const ChunkCoord& c) {
    coord = c;
    path = ChunkPath(); // Will be set via InitWithPath if needed
    std::fill(blocks.begin(), blocks.end(), 0);
    quads.clear();
}

void Chunk::InitWithPath(const ChunkPath& p) {
    path = p;
    coord = {0, 0, 0};
    std::fill(blocks.begin(), blocks.end(), 0);
    quads.clear();

    const auto& steps = path.GetPath();
    if (steps.empty()) {
        return;
    }

    // Derive coordinates from the path: each step encodes x,y,z offsets
    // x: -4 to 3, y: -4 to 3, z: -2 to 1; accumulate in base-8 for unique positions
    for (uint8_t encoded : steps) {
        int x, y, z;
        ChunkPath::DecodeTriple(encoded, x, y, z);

        coord.x = coord.x * 8 + x;
        coord.y = coord.y * 8 + y;
        coord.z = coord.z * 8 + z;
    }
}

std::string Chunk::GetIdentifier() const {
    if (!path.IsRoot()) {
        return path.ToHexString();
    }
    // Fallback to coordinate-based identifier
    return "chunk_" + std::to_string(coord.x) + "_" + std::to_string(coord.y) + "_" + std::to_string(coord.z);
}

BlockId Chunk::Get(int x, int y, int z) const {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE || z < 0 || z >= SIZE) return 0;
    int idx = x + z * SIZE + y * SIZE * SIZE;
    return blocks[idx];
}

void Chunk::Set(int x, int y, int z, BlockId id) {
    if (x < 0 || x >= SIZE || y < 0 || y >= SIZE || z < 0 || z >= SIZE) return;
    int idx = x + z * SIZE + y * SIZE * SIZE;
    blocks[idx] = id;
}

bool Chunk::Save(const std::string& basePath) const {
    try {
        // Create directory structure if needed
        fs::path dirPath(basePath);
        if (!fs::exists(dirPath)) {
            fs::create_directories(dirPath);
        }

        // Generate filename from identifier
        std::string filename = GetIdentifier() + ".chunk";
        fs::path filepath = dirPath / filename;

        // Open file and write block data
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            Log::Error("Chunk::Save - Failed to open file: " + filepath.string());
            return false;
        }

        // Write header (magic number + version)
        uint32_t magic = 0xDEADBEEF;
        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));

        // Write block data
        size_t blockCount = blocks.size();
        file.write(reinterpret_cast<const char*>(&blockCount), sizeof(blockCount));
        file.write(reinterpret_cast<const char*>(blocks.data()), blockCount * sizeof(BlockId));

        // Write coordinate and path info for validation
        file.write(reinterpret_cast<const char*>(&coord.x), sizeof(coord.x));
        file.write(reinterpret_cast<const char*>(&coord.y), sizeof(coord.y));
        file.write(reinterpret_cast<const char*>(&coord.z), sizeof(coord.z));

        file.close();
        Log::Info("Chunk::Save - Saved chunk " + GetIdentifier() + " to " + filepath.string());
        return true;
    }
    catch (const std::exception& e) {
        Log::Error("Chunk::Save - Exception: " + std::string(e.what()));
        return false;
    }
}

bool Chunk::Load(const std::string& basePath) {
    try {
        // Generate filename from identifier
        std::string filename = GetIdentifier() + ".chunk";
        fs::path filepath(basePath);
        filepath /= filename;

        if (!fs::exists(filepath)) {
            Log::Warning("Chunk::Load - File not found: " + filepath.string());
            return false;
        }

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            Log::Error("Chunk::Load - Failed to open file: " + filepath.string());
            return false;
        }

        // Read and verify header
        uint32_t magic = 0;
        uint32_t version = 0;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        file.read(reinterpret_cast<char*>(&version), sizeof(version));

        if (magic != 0xDEADBEEF || version != 1) {
            Log::Error("Chunk::Load - Invalid header (magic: " + std::to_string(magic) + ", version: " + std::to_string(version) + ")");
            file.close();
            return false;
        }

        // Read block data
        size_t blockCount = 0;
        file.read(reinterpret_cast<char*>(&blockCount), sizeof(blockCount));

        if (blockCount != blocks.size()) {
            Log::Error("Chunk::Load - Block count mismatch (expected " + std::to_string(blocks.size()) + ", got " + std::to_string(blockCount) + ")");
            file.close();
            return false;
        }

        file.read(reinterpret_cast<char*>(blocks.data()), blockCount * sizeof(BlockId));

        // Read coordinate info for validation (but don't fail if it mismatches)
        int savedX = 0, savedY = 0, savedZ = 0;
        file.read(reinterpret_cast<char*>(&savedX), sizeof(savedX));
        file.read(reinterpret_cast<char*>(&savedY), sizeof(savedY));
        file.read(reinterpret_cast<char*>(&savedZ), sizeof(savedZ));

        if (savedX != coord.x || savedY != coord.y || savedZ != coord.z) {
            Log::Warning("Chunk::Load - Coordinate mismatch (saved: " + std::to_string(savedX) + "," + std::to_string(savedY) + "," + std::to_string(savedZ) + 
                         ", current: " + std::to_string(coord.x) + "," + std::to_string(coord.y) + "," + std::to_string(coord.z) + ")");
        }

        file.close();
        Log::Info("Chunk::Load - Loaded chunk " + GetIdentifier() + " from " + filepath.string());
        return true;
    }
    catch (const std::exception& e) {
        Log::Error("Chunk::Load - Exception: " + std::string(e.what()));
        return false;
    }
}
