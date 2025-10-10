#pragma once
#include <string>
#include <fstream>

enum class LogLevel {
    Info,
    Warning,
    Error
};

class Log {
public:
    static void Init();
    static void Shutdown();

    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);

private:
    static void Write(LogLevel level, const std::string& message);
    static std::ofstream logfile;
};
