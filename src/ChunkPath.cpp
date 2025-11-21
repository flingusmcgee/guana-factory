#include "ChunkPath.h"
#include <sstream>
#include <iomanip>

ChunkPath::ChunkPath(const std::vector<uint8_t>& pathIndices)
    : path(pathIndices) {
}

uint8_t ChunkPath::EncodeTriple(int x, int y, int z) {
    // Clamp to ranges: x -4 to 3 (3 bits), y -4 to 3 (3 bits), z -2 to 1 (2 bits)
    x = (x + 4) & 0x7;  // 3 bits
    y = (y + 4) & 0x7;  // 3 bits
    z = (z + 2) & 0x3;  // 2 bits
    
    return (uint8_t)((x << 5) | (y << 2) | z);
}

void ChunkPath::DecodeTriple(uint8_t encoded, int& x, int& y, int& z) {
    x = ((encoded >> 5) & 0x7) - 4;   // 3 bits
    y = ((encoded >> 2) & 0x7) - 4;   // 3 bits
    z = ((encoded >> 0) & 0x3) - 2;   // 2 bits
}

void ChunkPath::Append(int x, int y, int z) {
    path.push_back(EncodeTriple(x, y, z));
}

void ChunkPath::AppendEncoded(uint8_t encoded) {
    path.push_back(encoded);
}

std::string ChunkPath::ToHexString() const {
    if (path.empty()) {
        return "ROOT";
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) oss << "_";
        
        int x, y, z;
        DecodeTriple(path[i], x, y, z);
        
        // Hex output
        oss << std::hex << std::setfill('0')
            << std::setw(1) << x
            << std::setw(1) << y
            << std::setw(1) << z;
    }
    
    return oss.str();
}

ChunkPath ChunkPath::FromHexString(const std::string& hexStr) {
    if (hexStr == "ROOT") {
        return ChunkPath();
    }
    
    ChunkPath result;
    std::istringstream iss(hexStr);
    std::string token;
    
    while (std::getline(iss, token, '_')) {
        if (token.length() != 3) continue;
        
        int x = std::stoi(token.substr(0, 1), nullptr, 16);
        int y = std::stoi(token.substr(1, 1), nullptr, 16);
        int z = std::stoi(token.substr(2, 1), nullptr, 16);
        
        result.Append(x, y, z);
    }
    
    return result;
}

ChunkPath ChunkPath::GetParent() const {
    if (path.empty()) {
        return ChunkPath(); // Root has no parent
    }
    
    std::vector<uint8_t> parentPath(path.begin(), path.end() - 1);
    return ChunkPath(parentPath);
}

uint32_t ChunkPath::Hash() const {
    // Simple hash: combine all bytes
    uint32_t hash = 0;
    for (size_t i = 0; i < path.size(); ++i) {
        hash = hash * 31 + path[i];
    }
    return hash;
}

bool ChunkPath::operator==(const ChunkPath& other) const {
    return path == other.path;
}
