#include "Log.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include "Config.h"
// Platform mkdir
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

// Define the static members
std::ofstream Log::logfile;
std::ofstream Log::debugfile;
LogLevel Log::minLevel = LogLevel::Info;

// A private helper function to get a clean timestamp string
static std::string GetTimestamp() noexcept {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t in_time_t = system_clock::to_time_t(now);

    // Use thread-safe localtime variants where available and format with strftime
    std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
    // MSVC / Windows
    localtime_s(&tm, &in_time_t);
#else
    // POSIX
    localtime_r(&in_time_t, &tm);
#endif

    char time_buffer[64];
    if (std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &tm) == 0) {
        return std::string();
    }
    return std::string(time_buffer);
}

// Initialize the logging system and open the timestamped log file
void Log::Init() {
    std::string timestamp = GetTimestamp();
    // Replace colons in the timestamp as they are invalid in Windows filenames
    for (char &c : timestamp) {
        if (c == ':') {
            c = '-';
        }
    }
    std::string filename = "iguana - " + timestamp + ".log";
    std::string debugFilename = "iguana - " + timestamp + ".debug.log";

    // Allow overriding the log directory from config
    // Avoid std::filesystem to keep editor/tooling happy; use a simple string path
    std::string defaultLogDir = "build/log";
    std::string logDirStr = defaultLogDir;
    bool enableDebugFile = false;
    try {
        if (!Config::IsLoaded()) {
            // Config may not be loaded yet; attempt to load it once
            Config::Load("config.ini");
        }
        logDirStr = Config::GetString("log.dir", defaultLogDir);
        enableDebugFile = Config::GetBool("log.debug_file", false);
        
        // Parse log level from config
        std::string levelStr = Config::GetString("log.level", "Info");
        if (levelStr == "Debug") {
            minLevel = LogLevel::Debug;
        } else if (levelStr == "Info") {
            minLevel = LogLevel::Info;
        } else if (levelStr == "Warning") {
            minLevel = LogLevel::Warning;
        } else if (levelStr == "Error") {
            minLevel = LogLevel::Error;
        }
    } catch (...) {
        logDirStr = defaultLogDir;
        enableDebugFile = false;
    }

    try {
        // Ensure the directory exists (create recursively if needed) so the
        // logfile is created under the intended folder instead of the current
        // working directory.
        auto make_dirs = [](const std::string &path) {
            if (path.empty()) return;
            // Normalize separators to '/'
            std::string p = path;
            for (char &c : p) if (c == '\\') c = '/';
            // Remove trailing slash
            if (!p.empty() && p.back() == '/') p.pop_back();
            size_t pos = 0;
            while (true) {
                size_t next = p.find('/', pos);
                std::string sub = (next == std::string::npos) ? p : p.substr(0, next);
                if (!sub.empty()) {
#if defined(_WIN32) || defined(_WIN64)
                    _mkdir(sub.c_str());
#else
                    mkdir(sub.c_str(), 0755);
#endif
                }
                if (next == std::string::npos) break;
                pos = next + 1;
            }
        };

        std::string fullPath = logDirStr;
        if (!fullPath.empty() && fullPath.back() != '/' && fullPath.back() != '\\') fullPath += "/";
        // Create the directory tree
        if (!logDirStr.empty()) make_dirs(logDirStr);
        fullPath += filename;

        // Attempt to open the file at the requested location; if that fails,
        // fall back to the current directory.
        logfile.open(fullPath, std::ios::out | std::ios::trunc);
        if (!logfile.is_open()) {
            logfile.open(filename, std::ios::out | std::ios::trunc);
        }
        
        // Open debug file if enabled
        if (enableDebugFile) {
            std::string debugPath = logDirStr;
            if (!debugPath.empty() && debugPath.back() != '/' && debugPath.back() != '\\') debugPath += "/";
            debugPath += debugFilename;
            debugfile.open(debugPath, std::ios::out | std::ios::trunc);
            if (!debugfile.is_open()) {
                debugfile.open(debugFilename, std::ios::out | std::ios::trunc);
            }
        }
    } catch (...) {
        // fallback to current directory if anything unexpected happens
        logfile.open(filename, std::ios::out | std::ios::trunc);
        if (enableDebugFile) {
            debugfile.open(debugFilename, std::ios::out | std::ios::trunc);
        }
    }

    if (!logfile.is_open()) {
        std::cerr << "CRITICAL ERROR: Failed to open " << filename << " for writing" << std::endl;
    } else {
        // We log the initialization *after* we confirm the file is open
        Info("Log file '" + filename + "' initialized");
        if (enableDebugFile && debugfile.is_open()) {
            Info("Debug log file '" + debugFilename + "' initialized");
        }
    }
}

// Close the log file stream
void Log::Shutdown() {
    Info("Logging system shutting down");
    if (logfile.is_open()) {
        logfile.close();
    }
    if (debugfile.is_open()) {
        debugfile.close();
    }
}

// Public-facing functions for logging at different levels
void Log::Debug(const std::string& message) {
    Write(LogLevel::Debug, message);
}

void Log::Info(const std::string& message) {
    Write(LogLevel::Info, message);
}

void Log::Warning(const std::string& message) {
    Write(LogLevel::Warning, message);
}

void Log::Error(const std::string& message) {
    Write(LogLevel::Error, message);
}

void Log::SetLevel(LogLevel level) {
    minLevel = level;
}

LogLevel Log::GetLevel() {
    return minLevel;
}

// The core, private function that writes to all targets
void Log::Write(LogLevel level, const std::string& message) {
    // Skip if below minimum level
    if (level < minLevel) {
        return;
    }
    
    static std::mutex write_mutex;
    std::lock_guard<std::mutex> guard(write_mutex);

    std::string timestamp = GetTimestamp();
    std::string level_str;

    // Determine level string and console color
    switch (level) {
        case LogLevel::Debug:
            level_str = "[DEBUG]";
            std::cout << "\033[36m" << timestamp << " " << level_str << " " << message << "\033[0m" << std::endl;
            break;
        case LogLevel::Info:
            level_str = "[INFO]";
            std::cout << timestamp << " " << level_str << " " << message << std::endl;
            break;
        case LogLevel::Warning:
            level_str = "[WARNING]";
            std::cout << "\033[93m" << timestamp << " " << level_str << " " << message << "\033[0m" << std::endl;
            break;
        case LogLevel::Error:
            level_str = "[ERROR]";
            std::cerr << "\033[91m" << timestamp << " " << level_str << " " << message << "\033[0m" << std::endl;
            break;
    }

    // File Output (main log)
    if (logfile.is_open()) {
        logfile << timestamp << " " << level_str << " " << message << std::endl;
        logfile.flush();
    }
    
    // Debug file output (if enabled and this is a debug message)
    if (debugfile.is_open() && level == LogLevel::Debug) {
        debugfile << timestamp << " " << level_str << " " << message << std::endl;
        debugfile.flush();
    }
}
