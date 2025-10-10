#pragma once
#include "Archetype.h"
#include <string>
#include <map>

// Manager for loading and accessing archetypes
class ArchetypeManager {
public:
    static ArchetypeManager& GetInstance();

    void LoadArchetypesFromDirectory(const std::string& directoryPath);
    Archetype* GetArchetype(const std::string& name);

private:
    ArchetypeManager() {}
    // Private
    Archetype LoadFile(const std::string& filepath, const std::string& name);
    std::map<std::string, Archetype> archetypes;
};