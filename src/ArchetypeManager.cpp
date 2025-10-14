#include "ArchetypeManager.h"
#include "Log.h"
#include <fstream>
#include <string>

// Provide a single, global instance
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

// The core function that recursively parses a file given its full path
Archetype ArchetypeManager::LoadFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Log::Error("Failed to open archetype file: " + filepath);
        return Archetype(); // Return an empty archetype on failure
    }

    std::string name = GetFileNameWithoutExt(filepath.c_str());
    Log::Info("Parsing archetype: " + name);
    Archetype newArchetype;
    
    std::string line;
    while (std::getline(file, line)) {
        size_t delimiterPos = line.find(':');
        if (delimiterPos == std::string::npos || line[0] == '#') continue;

        std::string key = trim(line.substr(0, delimiterPos));
        std::string value = trim(line.substr(delimiterPos + 1));

        if (key == "inherits") {
            std::string parentName = value;
            // This is the newpath construction
            std::string parentFilepath = current_directory + "/" + parentName + ".archetype";
            
            if (archetypes.find(parentName) == archetypes.end()) {
                // Perform the recursive call with the full, correct path
                newArchetype = LoadFile(parentFilepath);
            } else {
                newArchetype = archetypes.at(parentName);
            }
        }
        // Overwrite or set fields
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

    archetypes[name] = newArchetype;
    file.close();
    return newArchetype;
}