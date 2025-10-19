// Lightweight stubs for file/path helpers used by the engine.

#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <cctype>
#include <cstdio>
#include "include/raylib.h"

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

// Platform-specific directory listing implementations
#if defined(_WIN32)
#include <io.h>
#include <sys/stat.h>

static FilePathList LoadDirectoryFiles(const char *dirPath) {
    FilePathList list{};
    if (!dirPath) return list;
    std::string pattern = std::string(dirPath) + "\\*";
    struct _finddata_t fd;
    intptr_t h = _findfirst(pattern.c_str(), &fd);
    if (h == -1) return list;
    std::vector<std::string> files;
    do {
        if (!(fd.attrib & _A_SUBDIR)) {
            files.push_back(std::string(dirPath) + "\\" + fd.name);
        }
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
    list.count = static_cast<unsigned int>(files.size());
    list.capacity = list.count;
    if (list.count > 0) {
        list.paths = (char**)std::malloc(sizeof(char*) * list.count);
        for (unsigned int i = 0; i < list.count; ++i) list.paths[i] = alloc_cstr(files[i]);
    } else list.paths = nullptr;
    return list;
}

#else
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

static FilePathList LoadDirectoryFiles(const char *dirPath) {
    FilePathList list{};
    if (!dirPath) return list;
    DIR *d = opendir(dirPath);
    if (!d) return list;
    std::vector<std::string> files;
    struct dirent *entry;
    while ((entry = readdir(d)) != nullptr) {
        // Skip '.' and '..'
        if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0) continue;
        std::string p = std::string(dirPath) + "/" + entry->d_name;
        struct stat st; if (stat(p.c_str(), &st) == 0 && S_ISREG(st.st_mode)) files.push_back(p);
    }
    closedir(d);
    list.count = static_cast<unsigned int>(files.size());
    list.capacity = list.count;
    if (list.count > 0) {
        list.paths = (char**)std::malloc(sizeof(char*) * list.count);
        for (unsigned int i = 0; i < list.count; ++i) list.paths[i] = alloc_cstr(files[i]);
    } else list.paths = nullptr;
    return list;
}
#endif

static void UnloadDirectoryFiles(FilePathList files) {
    if (!files.paths) return;
    for (unsigned int i = 0; i < files.count; ++i) if (files.paths[i]) std::free(files.paths[i]);
    std::free(files.paths);
}

static const char *GetFileNameWithoutExt(const char *filePath) {
    static thread_local std::string s;
    if (!filePath) { s.clear(); return s.c_str(); }
    std::string p(filePath);
    size_t pos = p.find_last_of("/\\");
    std::string fname = (pos == std::string::npos) ? p : p.substr(pos + 1);
    size_t dot = fname.find_last_of('.');
    if (dot == std::string::npos) s = fname; else s = fname.substr(0, dot);
    return s.c_str();
}
