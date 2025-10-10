#pragma once
#include "include/raylib.h"
#include <string>

// The base object for everything in the game world
struct Entity {
    unsigned int id = 0;
    bool is_active = false;
    bool needs_to_die = false;
    std::string tag;

    Vector3 position = {};
    Vector3 velocity = {}; 
    BoundingBox bounds = {};
    Model model;
    Color color = WHITE;
};