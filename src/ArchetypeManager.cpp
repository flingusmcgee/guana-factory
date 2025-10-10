#include "ArchetypeManager.h"
#include "Log.h"
#include <fstream>
#include <string>
#include <algorithm>
ArchetypeManager& ArchetypeManager::GetInstance() {
    static ArchetypeManager instance;
    return instance;
}

// Helper function to trim leading and trailing whitespace from a string
static std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        ++start;
    }
    auto end = s.end();
    do {
        --end;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

// Scans a directory and loads all .archetype files within it
void ArchetypeManager::LoadArchetypesFromDirectory(const std::string& directoryPath) {
    Log::Info("Scanning for archetypes in: " + directoryPath);
    FilePathList files = LoadDirectoryFiles(directoryPath.c_str());

    for (unsigned int i = 0; i < files.count; i++) {
        const char* path = files.paths[i];
        if (IsFileExtension(path, ".archetype")) {
            // Extract the name from the filepath
            std::string name = GetFileNameWithoutExt(path);
            // Check if we haven't already loaded this archetype as a parent
            if (archetypes.find(name) == archetypes.end()) {
                LoadFile(path, name);
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

// The core function that recursively parses a file
Archetype ArchetypeManager::LoadFile(const std::string& filepath, const std::string& name) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Log::Error("Failed to open archetype file: " + filepath);
        return Archetype(); // Return an empty archetype on failure
    }

    Log::Info("Parsing archetype: " + name);
    Archetype newArchetype;
    
    std::string line;
    while (std::getline(file, line)) {
        size_t delimiterPos = line.find(':');
        if (delimiterPos == std::string::npos || line[0] == '#') continue;

        std::string key = trim(line.substr(0, delimiterPos));
        std::string value = trim(line.substr(delimiterPos + 1));

        // This is the recursive inheritance logic
        if (key == "inherits") {
            // If this archetype inherits, first load the parent
            std::string parentName = value;
            std::string parentFilepath = "../res/archetypes/" + parentName + ".archetype";
            
            // Check if we've already loaded the parent
            if (archetypes.find(parentName) == archetypes.end()) {
                // If not, load it now
                newArchetype = LoadFile(parentFilepath, parentName);
            } else {
                // If we have just copy its data
                newArchetype = archetypes.at(parentName);
            }
        }
        // OVERRIDE OR SET FIELDS
        else if (key == "tag") newArchetype.tag = value;
        else if (key == "model_id") newArchetype.model_id = value;
        else if (key == "color_r") newArchetype.color.r = (unsigned char)std::stoi(value);
        else if (key == "color_g") newArchetype.color.g = (unsigned char)std::stoi(value);
        else if (key == "color_b") newArchetype.color.b = (unsigned char)std::stoi(value);
        else if (key == "color_a") newArchetype.color.a = (unsigned char)std::stoi(value);
        else if (key == "velocity_x") newArchetype.velocity.x = std::stof(value);
        else if (key == "velocity_y") newArchetype.velocity.y = std::stof(value);
        else if (key == "velocity_z") newArchetype.velocity.z = std::stof(value);
    }

    // Store the merged archetype
    archetypes[name] = newArchetype;
    file.close();
    return newArchetype;
}