#include "Input.h"
#include "Config.h"
#include "include/raylib.h"
#include <algorithm>
#include <sstream>

namespace {
    struct ActionState {
        std::vector<int> keys;
        bool down = false;
        bool pressed = false;
        bool released = false;
    };

    std::map<std::string, ActionState> actions;

    std::vector<std::string> split(const std::string& s, char sep) {
        std::vector<std::string> out;
        std::istringstream ss(s);
        std::string item;
        while (std::getline(ss, item, sep)) {
            // trim
            size_t a = 0; while (a < item.size() && std::isspace(static_cast<unsigned char>(item[a]))) ++a;
            size_t b = item.size(); while (b > a && std::isspace(static_cast<unsigned char>(item[b-1]))) --b;
            if (b > a) out.push_back(item.substr(a, b-a));
        }
        return out;
    }
}

void Input::Init() {
    actions.clear();
    BindDefault();
    LoadFromConfig();
}

void Input::BindDefault() {
    // defaults
    actions["move_forward"].keys = { KEY_W, KEY_UP };
    actions["move_back"].keys = { KEY_S, KEY_DOWN };
    actions["move_left"].keys = { KEY_A, KEY_LEFT };
    actions["move_right"].keys = { KEY_D, KEY_RIGHT };
    actions["move_up"].keys = { KEY_SPACE };
    actions["move_down"].keys = { KEY_LEFT_CONTROL };
    actions["sprint"].keys = { KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT };
    actions["toggle_cursor"].keys = { KEY_TAB };
    actions["quit"].keys = { KEY_ESCAPE };
    actions["debug_toggle"].keys = { KEY_F3 };
}

void Input::LoadFromConfig() {
    if (!Config::IsLoaded()) return;
    // example config keys: input.move_forward = W,Up
    for (auto &kv : std::vector<std::pair<std::string,std::string>>{
        {"move_forward","input.move_forward"},
        {"move_back","input.move_back"},
        {"move_left","input.move_left"},
        {"move_right","input.move_right"},
        {"move_up","input.move_up"},
        {"move_down","input.move_down"},
        {"sprint","input.sprint"},
        {"toggle_cursor","input.toggle_cursor"},
        {"quit","input.quit"},
        {"debug_toggle","input.debug_toggle"}
    }) {
        std::string action = kv.first;
        std::string cfg = Config::GetString(kv.second, std::string());
        if (!cfg.empty()) {
            auto parts = split(cfg, ',');
            actions[action].keys.clear();
            for (auto &p : parts) {
                int k = Input::StringToKey(p);
                if (k != -1) actions[action].keys.push_back(k);
            }
        }
    }
}

void Input::Update() {
    for (auto &it : actions) {
        ActionState &s = it.second;
        bool anyDown = false;
        for (int k : s.keys) {
            if (IsKeyDown(k)) { anyDown = true; break; }
        }
        bool anyPressed = false;
        for (int k : s.keys) {
            if (IsKeyPressed(k)) { anyPressed = true; break; }
        }
        bool anyReleased = false;
        for (int k : s.keys) {
            if (IsKeyReleased(k)) { anyReleased = true; break; }
        }
        s.pressed = anyPressed;
        s.released = anyReleased;
        s.down = anyDown;
    }
}

bool Input::IsDown(const std::string& action) {
    auto it = actions.find(action);
    if (it == actions.end()) return false;
    return it->second.down;
}

bool Input::WasPressed(const std::string& action) {
    auto it = actions.find(action);
    if (it == actions.end()) return false;
    return it->second.pressed;
}

bool Input::WasReleased(const std::string& action) {
    auto it = actions.find(action);
    if (it == actions.end()) return false;
    return it->second.released;
}

// VERY small map of names to raylib keys; extend as needed please or get molested by unrecognized key
int Input::StringToKey(const std::string& name) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c){ return std::toupper(c); });
    if (n == "W") return KEY_W;
    if (n == "A") return KEY_A;
    if (n == "S") return KEY_S;
    if (n == "D") return KEY_D;
    if (n == "UP") return KEY_UP;
    if (n == "DOWN") return KEY_DOWN;
    if (n == "LEFT") return KEY_LEFT;
    if (n == "RIGHT") return KEY_RIGHT;
    if (n == "SPACE") return KEY_SPACE;
    if (n == "CTRL" || n == "LCTRL" || n == "LEFT_CONTROL") return KEY_LEFT_CONTROL;
    if (n == "LSHIFT" || n == "SHIFT") return KEY_LEFT_SHIFT;
    if (n == "RSHIFT" || n == "RIGHT_SHIFT") return KEY_RIGHT_SHIFT;
    if (n == "TAB") return KEY_TAB;
    if (n == "ESC" || n == "ESCAPE") return KEY_ESCAPE;
    if (n == "F3") return KEY_F3;
    return -1;
}
