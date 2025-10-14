#pragma once
#include "Archetype.h"
#include <string>
#include <map>

// Manager for loading and accessing archetypes
class ArchetypeManager {
public:
    static ArchetypeManager& GetInstance();

    // The public-facing entry point
    void LoadArchetypesFromDirectory(const std::string& directoryPath);
    Archetype* GetArchetype(const std::string& name);

private:
    ArchetypeManager() {}

    // The private parser, dumb and stupid
    Archetype LoadFile(const std::string& filepath);
    
    std::map<std::string, Archetype> archetypes;
    std::string current_directory; // Stores the directory
};