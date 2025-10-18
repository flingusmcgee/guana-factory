#pragma once
#include "Archetype.h"
#include "include/raylib.h"
#include <string>
#include <cstddef>
#include <map>
#include <unordered_map>
#include <unordered_set>

// Manager for loading and accessing archetypes
class ArchetypeManager {
public:
    static ArchetypeManager& GetInstance();

    // The public-facing entry point
    void LoadArchetypesFromDirectory(const std::string& directoryPath);
    Archetype* GetArchetype(const std::string& name);
    // Return the number of loaded archetypes
    size_t GetLoadedCount() const;

private:
    ArchetypeManager() {}

    // The private parser, dumb and stupid
    Archetype LoadFile(const std::string& filepath);
    // Loads an archetype into the internal map and returns true on success
    bool LoadFileToMap(const std::string& filepath);
    // Internal implementation used to track recursive loads and detect cycles
    Archetype LoadFileInternal(const std::string& filepath, std::unordered_set<std::string>& loading);
    
    std::unordered_map<std::string, Archetype> archetypes;
    std::string current_directory; // Stores the directory
};