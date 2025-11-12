#pragma once
#include "include/raylib.h"
#include <string>
#include <map>
#include <unordered_map>

// A static class for loading and accessing all game assets
class AssetManager {
public:
    static void LoadAssets();
    static void UnloadAssets();
    static Model& GetModel(const std::string& id);
    static Texture2D& GetTexture(const std::string& id);
    static bool ModelExists(const std::string& id); // New! Check if a model exists

private:
    static std::unordered_map<std::string, Model> models;
    static std::unordered_map<std::string, Texture2D> textures;
};