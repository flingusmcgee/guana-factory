// Lightweight C-linkage stubs for a small subset of raylib file helpers
// These are intended for non-graphical builds (tests/CLI) where full raylib
// is not linked. They provide minimal, portable behavior.

#include "include/raylib.h"
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>

static inline std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static char *alloc_cstr(const std::string &s) {
    char *p = (char*)std::malloc(s.size() + 1);
    if (!p) return nullptr;
    std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

extern "C" {

bool IsFileExtension(const char *fileName, const char *ext) {
    if (!fileName || !ext) return false;
    std::string f = to_lower(std::string(fileName));
    std::string e = to_lower(std::string(ext));
    if (e.size() == 0) return true;
    if (e[0] != '.') e = std::string(".") + e;
    if (f.size() < e.size()) return false;
    return f.compare(f.size() - e.size(), e.size(), e) == 0;
}

FilePathList LoadDirectoryFiles(const char *dirPath) {
    FilePathList list{};
    if (!dirPath) return list;
    try {
        std::vector<std::string> files;
        for (auto &entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) files.push_back(entry.path().string());
        }
        list.count = static_cast<unsigned int>(files.size());
        list.capacity = list.count;
        if (list.count > 0) {
            list.paths = (char**)std::malloc(sizeof(char*) * list.count);
            for (unsigned int i = 0; i < list.count; ++i) {
                list.paths[i] = alloc_cstr(files[i]);
            }
        } else {
            list.paths = nullptr;
        }
    } catch (...) {
        list.count = 0; list.capacity = 0; list.paths = nullptr;
    }
    return list;
}

void UnloadDirectoryFiles(FilePathList files) {
    if (!files.paths) return;
    for (unsigned int i = 0; i < files.count; ++i) {
        if (files.paths[i]) std::free(files.paths[i]);
    }
    std::free(files.paths);
}

const char *GetFileNameWithoutExt(const char *filePath) {
    static thread_local std::string s;
    if (!filePath) { s.clear(); return s.c_str(); }
    try {
        std::filesystem::path p(filePath);
        s = p.stem().string();
    } catch (...) { s.clear(); }
    return s.c_str();
}

} // extern "C"
