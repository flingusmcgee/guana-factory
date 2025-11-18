#pragma once
#include <string>
#include <fstream>

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Log {
public:
    static void Init();
    static void Shutdown();

    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);
    
    // Set minimum log level to display (default: Info)
    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();

private:
    static void Write(LogLevel level, const std::string& message);
    static std::ofstream logfile;
    static std::ofstream debugfile;
    static LogLevel minLevel;
};
