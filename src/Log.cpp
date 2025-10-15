#include "Log.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>

// Define the static logfile member
std::ofstream Log::logfile;

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
    
    // Command the one, true, static scribe to open the file
    logfile.open(filename, std::ios::out | std::ios::trunc);

    if (!logfile.is_open()) {
        std::cerr << "CRITICAL ERROR: Failed to open " << filename << " for writing" << std::endl;
    } else {
        // We log the initialization *after* we confirm the file is open
        Info("Log file '" + filename + "' initialized");
    }
}

// Close the log file stream
void Log::Shutdown() {
    Info("Logging system shutting down");
    if (logfile.is_open()) {
        logfile.close();
    }
}

// Public-facing function for information-level logs
void Log::Info(const std::string& message) {
    Write(LogLevel::Info, message);
}

// Public-facing function for warning-level logs
void Log::Warning(const std::string& message) {
    Write(LogLevel::Warning, message);
}

// Public-facing function for error-level logs
void Log::Error(const std::string& message) {
    Write(LogLevel::Error, message);
}

// The core, private function that writes to all targets
void Log::Write(LogLevel level, const std::string& message) {
    static std::mutex write_mutex;
    std::lock_guard<std::mutex> guard(write_mutex);

    std::string timestamp = GetTimestamp();
    std::string level_str;

    // Console Output
    switch (level) {
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

    // File Output
    if (logfile.is_open()) {
        logfile << timestamp << " " << level_str << " " << message << std::endl;
    }
}
