#include "PerlinNoise.h"
#include <algorithm>
#include <random>

PerlinNoise::PerlinNoise(unsigned int seed) {
    // Initialize permutation table
    p.resize(256);
    for (int i = 0; i < 256; ++i) {
        p[i] = i;
    }

    // Shuffle with seed
    std::mt19937 gen(seed);
    std::shuffle(p.begin(), p.end(), gen);

    // Duplicate for overflow
    p.insert(p.end(), p.begin(), p.end());
}

double PerlinNoise::fade(double t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) const {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y) const {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise(double x, double y, int octaves, double persistence) const {
    double value = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double maxValue = 0.0;

    for (int i = 0; i < octaves; ++i) {
        value += perlinNoise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }

    return value / maxValue;
}

double PerlinNoise::perlinNoise(double x, double y) const {
    // Find unit square
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;

    // Find relative x,y in square
    x -= std::floor(x);
    y -= std::floor(y);

    // Compute fade curves
    double u = fade(x);
    double v = fade(y);

    // Hash coordinates
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    // Add blended results from 4 corners
    double res = lerp(v,
        lerp(u, grad(p[AA], x, y), grad(p[BA], x - 1, y)),
        lerp(u, grad(p[AB], x, y - 1), grad(p[BB], x - 1, y - 1)));

    return (res + 1.0) / 2.0; // Normalize to [0,1]
}