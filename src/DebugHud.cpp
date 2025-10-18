#include "DebugHud.h"
#include "include/raylib.h"
#include <sstream>

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

void DebugHud::Draw(int fps, int entities, const std::string& extra) {
    if (!g_visible) return;
    std::stringstream ss;
    ss << "FPS: " << fps << "\n";
    ss << "Entities: " << entities << "\n";
    if (!extra.empty()) ss << extra << "\n";

    DrawText(ss.str().c_str(), 10, 30, 14, BLACK);
}
