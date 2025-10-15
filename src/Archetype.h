#pragma once
#include "include/raylib.h"
#include <string>

// A data container for an entity
struct Archetype {
    std::string tag;
    std::string model_id;
    Color color = WHITE;
    Vector3 velocity = {};
    std::string source_path; // file path this archetype was loaded from (for diagnostics)
};