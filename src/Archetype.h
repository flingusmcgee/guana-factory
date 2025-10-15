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
    // Internal flag that indicates whether this archetype was populated by parsing
    // or by merging with a parent. This is explicit (and safer) than inferring
    // emptiness from default color/velocity values.
    bool populated = false;

    // Returns true if this archetype has no meaningful data (not populated)
    bool isEmpty() const { return !populated; }
};