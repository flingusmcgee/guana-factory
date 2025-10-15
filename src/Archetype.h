#pragma once
#include "include/raylib.h"
#include <string>

// A data container for an entity
struct Archetype {
    std::string tag;
    std::string model_id;
    Color color = WHITE;
    Vector3 velocity = {};
    std::string source_path;
    
    bool populated = false;

    // Returns true if this archetype has no meaningful data (not populated)
    bool isEmpty() const { return !populated; }
};