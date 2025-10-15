#include "ArchetypeManager.h"
#include "Log.h"
#include "include/raylib.h"
#include <fstream>
#include <string>
#include <cctype>
#include <filesystem>

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
            std::filesystem::path p(path);
            std::string name = p.stem().string();
            // Check prevents reloading an archetype that was already loaded as a parent
            if (archetypes.find(name) == archetypes.end()) {
                LoadFileToMap(p.string());
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

bool ArchetypeManager::LoadFileToMap(const std::string& filepath) {
    Archetype a = LoadFile(filepath);
    if (a.isEmpty()) {
        return false;
    }
    std::filesystem::path p(filepath);
    std::string name = p.stem().string();
    // Prefer the archetype's tag as the map key when available (so files like cube_base.archetype
    // that declare tag: cube are stored under 'cube' as tests expect).
    if (!a.tag.empty()) {
        name = a.tag;
    }
    a.source_path = p.string();
    archetypes[name] = a;
    return true;
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

    // If we're already loading this archetype, abort to avoid cycles
    if (loading.find(name) != loading.end()) {
        Log::Error("Detected cyclic inheritance while loading: " + name);
        return Archetype();
    }

    // Temporary containers: child values and flags to indicate which fields were explicitly set by the child
    Archetype child;
    struct Flags {
        bool tag=false;
        bool model_id=false;
        bool color_r=false;
        bool color_g=false;
        bool color_b=false;
        bool color_a=false;
        bool vel_x=false;
        bool vel_y=false;
        bool vel_z=false;
    } flags;

    std::string parentName;
    std::string line;
    while (std::getline(file, line)) {
        std::string tline = trim(line);
        if (tline.empty() || tline[0] == '#') continue;

        size_t delimiterPos = tline.find(':');
        if (delimiterPos == std::string::npos) continue;

        std::string key = trim(tline.substr(0, delimiterPos));
        std::string value = trim(tline.substr(delimiterPos + 1));

        if (key == "inherits") {
            parentName = value;
        }
        else if (key == "tag") { child.tag = value; flags.tag = true; }
        else if (key == "model_id") { child.model_id = value; flags.model_id = true; }
        else if (key == "color_r") {
            try { child.color.r = static_cast<unsigned char>(std::stoi(value)); flags.color_r = true; } catch (...) { Log::Warning("Invalid color_r for " + name + ", value='" + value + "'"); }
        }
        else if (key == "color_g") {
            try { child.color.g = static_cast<unsigned char>(std::stoi(value)); flags.color_g = true; } catch (...) { Log::Warning("Invalid color_g for " + name + ", value='" + value + "'"); }
        }
        else if (key == "color_b") {
            try { child.color.b = static_cast<unsigned char>(std::stoi(value)); flags.color_b = true; } catch (...) { Log::Warning("Invalid color_b for " + name + ", value='" + value + "'"); }
        }
        else if (key == "color_a") {
            try { child.color.a = static_cast<unsigned char>(std::stoi(value)); flags.color_a = true; } catch (...) { Log::Warning("Invalid color_a for " + name + ", value='" + value + "'"); }
        }
        else if (key == "velocity_x") {
            try { child.velocity.x = std::stof(value); flags.vel_x = true; } catch (...) { Log::Warning("Invalid velocity_x for " + name + ", value='" + value + "'"); }
        }
        else if (key == "velocity_y") {
            try { child.velocity.y = std::stof(value); flags.vel_y = true; } catch (...) { Log::Warning("Invalid velocity_y for " + name + ", value='" + value + "'"); }
        }
        else if (key == "velocity_z") {
            try { child.velocity.z = std::stof(value); flags.vel_z = true; } catch (...) { Log::Warning("Invalid velocity_z for " + name + ", value='" + value + "'"); }
        }
    }

    file.close();

    // Now perform loading/merging. Mark this archetype as loading to detect cycles during parent load.
    loading.insert(name);

    Archetype result;
    // If we have a parent, try to load it (search in same directory as this file)
    if (!parentName.empty()) {
        // Prefer already-loaded parent
        if (archetypes.find(parentName) != archetypes.end()) {
            result = archetypes.at(parentName);
        } else {
            std::filesystem::path parentPath = p.parent_path() / (parentName + ".archetype");
            // Attempt to load the parent; if it fails, log the attempted path for easier debugging
            Archetype parent = LoadFileInternal(parentPath.string(), loading);
            if (parent.isEmpty()) {
                Log::Error("Failed to load parent archetype '" + parentName + "' for child '" + name + "'. Tried path: " + parentPath.string());
            }
            result = parent;
        }
    }

    // Merge: only apply child fields that were explicitly set
    if (flags.tag) result.tag = child.tag;
    if (flags.model_id) result.model_id = child.model_id;
    if (flags.color_r) result.color.r = child.color.r;
    if (flags.color_g) result.color.g = child.color.g;
    if (flags.color_b) result.color.b = child.color.b;
    if (flags.color_a) result.color.a = child.color.a;
    if (flags.vel_x) result.velocity.x = child.velocity.x;
    if (flags.vel_y) result.velocity.y = child.velocity.y;
    if (flags.vel_z) result.velocity.z = child.velocity.z;

    // Mark result as populated if either parent had data or child set any field
    bool childSetAny = flags.tag || flags.model_id || flags.color_r || flags.color_g || flags.color_b || flags.color_a || flags.vel_x || flags.vel_y || flags.vel_z;
    if (childSetAny || !parentName.empty() || !result.tag.empty() || !result.model_id.empty()) {
        result.populated = true;
    }

    // Finished loading this archetype
    loading.erase(name);

    archetypes[name] = result;
    return result;
}

size_t ArchetypeManager::GetLoadedCount() const {
    return archetypes.size();
}