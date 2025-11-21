#pragma once
#include <vector>
#include <string>
#include <cstdint>

/**
 * Immutable path to a chunk in the world hierarchy.
 * Like a file path, starts from root with child selections.
 * Unlike player-relative positions, this never changes with movement.
 * Example: "7a3_221" navigates through nested chunks.
 */
class ChunkPath {
public:
    ChunkPath() = default;
    explicit ChunkPath(const std::vector<uint8_t>& pathIndices);

    // Encode a single (x,y,z) triple into a byte for the path
    // x: -4 to 3 (3 bits), y: -4 to 3 (3 bits), z: -2 to 1 (2 bits)
    static uint8_t EncodeTriple(int x, int y, int z);
    
    // Decode a byte from path back into (x,y,z)
    static void DecodeTriple(uint8_t encoded, int& x, int& y, int& z);

    // Add a child chunk to the path (append a block coordinate)
    void Append(int x, int y, int z);
    void AppendEncoded(uint8_t encoded);

    // Get the hex string representation (e.g., "7a3_221")
    std::string ToHexString() const;

    // Parse from hex string (e.g., "7a3_221")
    static ChunkPath FromHexString(const std::string& hexStr);

    // Get depth (level) of this chunk in the tree
    int GetDepth() const { return path.size(); }

    // Get parent path (one level up)
    ChunkPath GetParent() const;

    // Compute a hash for fast lookup
    uint32_t Hash() const;

    // Comparison
    bool operator==(const ChunkPath& other) const;
    bool operator!=(const ChunkPath& other) const { return !(*this == other); }

    // Access underlying path
    const std::vector<uint8_t>& GetPath() const { return path; }

    // Check if this is the root path
    bool IsRoot() const { return path.empty(); }

private:
    std::vector<uint8_t> path; // Each byte encodes (x,y,z) triple
};
