#include "ArchetypeManager.h"
#include "Log.h"
#include <fstream>
#include <string>
#include <cctype>
#include <filesystem>
#include <iostream>

// Provide a single, global instance
ArchetypeManager& ArchetypeManager::GetInstance() {
    static ArchetypeManager instance;
    return instance;
}

// Helper function to trim leading and trailing whitespace from a string
static std::string trim(const std::string& s) {
    if (s.empty()) return std::string();
    auto start = s.begin();
    while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) ++start;
    if (start == s.end()) return std::string();
    auto end = s.end() - 1;
    while (end != start && std::isspace(static_cast<unsigned char>(*end))) --end;
    return std::string(start, end + 1);
}

// Scans a directory and commands the parser to load each file
void ArchetypeManager::LoadArchetypesFromDirectory(const std::string& directoryPath) {
    Log::Info("Scanning for archetypes in: " + directoryPath);
    current_directory = directoryPath; // Store the path for use in recursive calls
    FilePathList files = LoadDirectoryFiles(directoryPath.c_str());

    for (unsigned int i = 0; i < files.count; i++) {
        const char* path = files.paths[i];
        if (IsFileExtension(path, ".archetype")) {
            std::string name = GetFileNameWithoutExt(path);
            // Check prevents reloading an archetype that was already loaded as a parent
            if (archetypes.find(name) == archetypes.end()) {
                LoadFile(path);
            }
        }
    }
    UnloadDirectoryFiles(files);
}

// Get a pointer to a loaded archetype by its name
Archetype* ArchetypeManager::GetArchetype(const std::string& name) {
    if (archetypes.count(name)) {
        return &archetypes.at(name);
    }
    Log::Warning("Archetype not found: " + name);
    return nullptr;
}

// Public entry that initializes cycle detection state
Archetype ArchetypeManager::LoadFile(const std::string& filepath) {
    std::unordered_set<std::string> loading;
    return LoadFileInternal(filepath, loading);
}

// Internal implementation with cycle detection
Archetype ArchetypeManager::LoadFileInternal(const std::string& filepath, std::unordered_set<std::string>& loading) {
    std::filesystem::path p(filepath);
    std::string filepathStr = p.string();

    std::ifstream file(filepathStr);
    if (!file.is_open()) {
        Log::Error("Failed to open archetype file: " + filepathStr);
        return Archetype(); // Return an empty archetype on failure
    }

    std::string name = GetFileNameWithoutExt(filepathStr.c_str());
    Log::Info("Parsing archetype: " + name);

    // Cycle detection: if we're already loading this archetype, abort
    if (loading.find(name) != loading.end()) {
        Log::Error("Detected cyclic inheritance while loading: " + name);
        return Archetype();
    }
    loading.insert(name);

    Archetype newArchetype;
    std::string line;
    while (std::getline(file, line)) {
        std::string tline = trim(line);
        if (tline.empty() || tline[0] == '#') continue;

        size_t delimiterPos = tline.find(':');
        if (delimiterPos == std::string::npos) continue;

        std::string key = trim(tline.substr(0, delimiterPos));
        std::string value = trim(tline.substr(delimiterPos + 1));

        if (key == "inherits") {
            std::string parentName = value;
            std::filesystem::path parentPath = std::filesystem::path(current_directory) / (parentName + ".archetype");
            std::string parentFilepath = parentPath.string();

            if (archetypes.find(parentName) == archetypes.end()) {
                Archetype parent = LoadFileInternal(parentFilepath, loading);
                // merge parent into newArchetype if needed
                newArchetype = parent;
            } else {
                newArchetype = archetypes.at(parentName);
            }
        }
        // Overwrite or set fields with guards for numeric conversions
        else if (key == "tag") newArchetype.tag = value;
        else if (key == "model_id") newArchetype.model_id = value;
        else if (key == "color_r") {
            try { newArchetype.color.r = static_cast<unsigned char>(std::stoi(value)); } catch (...) { Log::Warning("Invalid color_r for " + name + ", value='" + value + "'"); }
        }
        else if (key == "color_g") {
            try { newArchetype.color.g = static_cast<unsigned char>(std::stoi(value)); } catch (...) { Log::Warning("Invalid color_g for " + name + ", value='" + value + "'"); }
        }
        else if (key == "color_b") {
            try { newArchetype.color.b = static_cast<unsigned char>(std::stoi(value)); } catch (...) { Log::Warning("Invalid color_b for " + name + ", value='" + value + "'"); }
        }
        else if (key == "color_a") {
            try { newArchetype.color.a = static_cast<unsigned char>(std::stoi(value)); } catch (...) { Log::Warning("Invalid color_a for " + name + ", value='" + value + "'"); }
        }
        else if (key == "velocity_x") {
            try { newArchetype.velocity.x = std::stof(value); } catch (...) { Log::Warning("Invalid velocity_x for " + name + ", value='" + value + "'"); }
        }
        else if (key == "velocity_y") {
            try { newArchetype.velocity.y = std::stof(value); } catch (...) { Log::Warning("Invalid velocity_y for " + name + ", value='" + value + "'"); }
        }
        else if (key == "velocity_z") {
            try { newArchetype.velocity.z = std::stof(value); } catch (...) { Log::Warning("Invalid velocity_z for " + name + ", value='" + value + "'"); }
        }
    }

    // Finished loading this archetype
    loading.erase(name);

    archetypes[name] = newArchetype;
    file.close();
    return newArchetype;
}

size_t ArchetypeManager::GetLoadedCount() const {
    return archetypes.size();
}