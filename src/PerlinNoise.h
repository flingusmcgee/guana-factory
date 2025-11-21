#pragma once
#include <vector>
#include <cmath>

// Simple Perlin noise implementation for terrain generation
class PerlinNoise {
public:
    PerlinNoise(unsigned int seed = 0);
    ~PerlinNoise() = default;

    // Generate noise value at (x, y) with given octaves and persistence
    double noise(double x, double y, int octaves = 4, double persistence = 0.5) const;

private:
    std::vector<int> p; // Permutation table

    double fade(double t) const;
    double lerp(double t, double a, double b) const;
    double grad(int hash, double x, double y) const;
    double perlinNoise(double x, double y) const;
};