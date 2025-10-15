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
    // Returns true if this archetype appears uninitialized / empty
    bool isEmpty() const {
        bool colorIsWhite = (color.r == WHITE.r && color.g == WHITE.g && color.b == WHITE.b && color.a == WHITE.a);
        bool velIsZero = (velocity.x == 0.0f && velocity.y == 0.0f && velocity.z == 0.0f);
        return tag.empty() && model_id.empty() && colorIsWhite && velIsZero;
    }
};