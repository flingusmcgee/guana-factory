#include "AssetManager.h"
#include "Config.h"
// Avoid <filesystem> to keep toolchain/editor warnings down
#include <sys/stat.h>

// Define the static maps
std::unordered_map<std::string, Model> AssetManager::models;
std::unordered_map<std::string, Texture2D> AssetManager::textures;

// Load all models and textures from disk
void AssetManager::LoadAssets() {
    models["cube"] = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    // Load and set window icon (avoid std::filesystem)
    auto file_exists = [](const std::string &p)->bool {
        struct stat buffer;
        return (stat(p.c_str(), &buffer) == 0);
    };
    std::string iconPath = Config::GetString("window.icon", "../res/icon.png");
    if (file_exists(iconPath)) {
        Image icon = LoadImage(iconPath.c_str());
        SetWindowIcon(icon);
        UnloadImage(icon);
    } else {
        // try relative fallback
        std::string alt = std::string("res/icon.png");
        if (file_exists(alt)) {
            Image icon = LoadImage(alt.c_str());
            SetWindowIcon(icon);
            UnloadImage(icon);
        }
    }
}

// Unload all assets from memory
void AssetManager::UnloadAssets() {
    for (auto it = models.begin(); it != models.end(); ++it) {
        UnloadModel(it->second);
    }
    for (auto it = textures.begin(); it != textures.end(); ++it) {
        UnloadTexture(it->second);
    }
    models.clear();
    textures.clear();
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
