#include "DebugHud.h"
#include "include/raylib.h"
#include <cstdio>
#include <cstring>

namespace {
    bool g_visible = false;
}

void DebugHud::Init() {
    g_visible = false;
}

void DebugHud::Toggle() {
    g_visible = !g_visible;
}

bool DebugHud::Visible() { return g_visible; }

void DebugHud::Draw(int entities, const char* extra) {
    if (!g_visible) return;
    char buf[512];
    int len = std::snprintf(buf, sizeof(buf), "Entities: %d\n", entities);
    if (extra != nullptr && extra[0] != '\0' && len < static_cast<int>(sizeof(buf) - 1)) {
        int more = std::snprintf(buf + len, sizeof(buf) - len, "%s\n", extra);
        if (more > 0) len += more;
    }
    DrawText(buf, 10, 30, 14, BLACK);
}
