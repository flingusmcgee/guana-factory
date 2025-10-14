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
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    char time_buffer[26];
    ctime_s(time_buffer, sizeof(time_buffer), &in_time_t);
    std::string timestamp(time_buffer);
    timestamp.pop_back(); // Remove the trailing newline character
    return timestamp;
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
