#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

#if defined(_WIN32) && defined(BUILD_SHARED_LIBS)
    #ifdef LOGGER_EXPORTS
        #define LOGGER_API __declspec(dllexport)
    #else
        #define LOGGER_API __declspec(dllimport)
    #endif
#else
    #define LOGGER_API
#endif


enum class LogLevel {
    LOW,    
    MEDIUM, 
    HIGH    
};

class Logger {
public:
    Logger() = delete;

    
    static bool init(const std::string& filename, LogLevel defaultLevel = LogLevel::MEDIUM);

    
    static void log(const std::string& message, LogLevel level);

    
    static void setLogLevel(LogLevel newLevel);

private:
    static Logger& getInstance();
    Logger(const std::string& filename, LogLevel defaultLevel);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    
    std::string levelToString(LogLevel level);

    std::ofstream m_logFile;
    LogLevel m_logLevel;
    std::mutex m_mutex;

    static Logger* m_instance;
    static std::mutex m_instanceMutex;
};

#endif // LOGGER_H