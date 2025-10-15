#include "AssetManager.h"
#include "Config.h"
#include <filesystem>

// Define the static maps
std::map<std::string, Model> AssetManager::models;
std::map<std::string, Texture2D> AssetManager::textures;

// Load all models and textures from disk
void AssetManager::LoadAssets() {
    models["cube"] = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    // Load and set the fuck boys
    std::string iconPath = Config::GetString("window.icon", "../res/icon.png");
    namespace fs = std::filesystem;
    if (fs::exists(iconPath)) {
        Image icon = LoadImage(iconPath.c_str());
        SetWindowIcon(icon);
        UnloadImage(icon);
    } else {
        // try relative fallback
        std::string alt = std::string("res/icon.png");
        if (fs::exists(alt)) {
            Image icon = LoadImage(alt.c_str());
            SetWindowIcon(icon);
            UnloadImage(icon);
        }
    }
}

// Unload all assets from memory
void AssetManager::UnloadAssets() {
    for (auto const& [id, model] : models) {
        UnloadModel(model);
    }
    for (auto const& [id, texture] : textures) {
        UnloadTexture(texture);
    }
}

// Get a loaded model by its ID
Model& AssetManager::GetModel(const std::string& id) {
    return models.at(id);
}

// Get a loaded texture by its ID
Texture2D& AssetManager::GetTexture(const std::string& id) {
    return textures.at(id);
}

bool AssetManager::ModelExists(const std::string& id) {
    return models.count(id) > 0;
}
