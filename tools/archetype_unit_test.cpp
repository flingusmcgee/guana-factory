#include <iostream>
#include <fstream>
#include "src/ArchetypeManager.cpp" // include implementation to keep this test standalone

int main() {
    // Create a temporary archetype file in res/archetypes
    const char* path = "res/archetypes/temp_white_zero.archetype";
    std::ofstream out(path);
    out << "tag: temp_white" << std::endl;
    out << "model_id: cube" << std::endl;
    out << "color_r: 255" << std::endl;
    out << "color_g: 255" << std::endl;
    out << "color_b: 255" << std::endl;
    out << "color_a: 255" << std::endl;
    out << "velocity_x: 0.0" << std::endl;
    out << "velocity_y: 0.0" << std::endl;
    out << "velocity_z: 0.0" << std::endl;
    out.close();

    ArchetypeManager::GetInstance().LoadArchetypesFromDirectory("res/archetypes");
    Archetype* a = ArchetypeManager::GetInstance().GetArchetype("temp_white");
    if (!a) {
        std::cerr << "FAILED: archetype not found" << std::endl;
        return 2;
    }
    if (a->isEmpty()) {
        std::cerr << "FAILED: archetype considered empty when it explicitly set white color and zero velocity" << std::endl;
        return 3;
    }
    std::cout << "PASS: archetype populated" << std::endl;
    return 0;
}
