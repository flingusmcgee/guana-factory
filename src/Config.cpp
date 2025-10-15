#include "Config.h"
#include <fstream>
#include <algorithm>
#include <cctype>

std::unordered_map<std::string, std::string> Config::values;

std::string Config::trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end-1]))) --end;
    return s.substr(start, end - start);
}

bool Config::Load(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        values[key] = value;
    }
    return true;
}

bool Config::GetBool(const std::string& key, bool defaultValue) {
    auto it = values.find(key);
    if (it == values.end()) return defaultValue;
    std::string v = it->second;
    for (auto &c : v) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return (v == "1" || v == "true" || v == "yes" || v == "on");
}

int Config::GetInt(const std::string& key, int defaultValue) {
    auto it = values.find(key);
    if (it == values.end()) return defaultValue;
    try { return std::stoi(it->second); } catch (...) { return defaultValue; }
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue) {
    auto it = values.find(key);
    if (it == values.end()) return defaultValue;
    return it->second;
}
