#pragma once
#include <string>
#include <unordered_map>

// Very small INI-like config loader used for de bug, paths, etc.
class Config {
public:
    // Load a simple key=value config file. Returns true on success.
    static bool Load(const std::string& path);

    // Accessors with defaults
    static bool GetBool(const std::string& key, bool defaultValue);
    static int GetInt(const std::string& key, int defaultValue);
    static float GetFloat(const std::string& key, float defaultValue);
    static std::string GetString(const std::string& key, const std::string& defaultValue);

private:
    static std::unordered_map<std::string, std::string> values;
    static std::string trim(const std::string& s);
};
