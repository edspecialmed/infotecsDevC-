#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime> 


Logger* Logger::m_instance = nullptr;
std::mutex Logger::m_instanceMutex;

bool Logger::init(const std::string& filename, LogLevel defaultLevel) {
    std::lock_guard<std::mutex> lock(m_instanceMutex);
    if (m_instance == nullptr) {
        try {
            m_instance = new Logger(filename, defaultLevel);
        }
        catch (const std::ofstream::failure& e) {
            std::cerr << "Error opening log file: " << e.what() << std::endl;
            return false;
        }
    }
    return true;
}

Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(m_instanceMutex);
    if (m_instance == nullptr) {
        throw std::runtime_error("Logger has not been initialized. Call Logger::init() first.");
    }
    return *m_instance;
}

Logger::Logger(const std::string& filename, LogLevel defaultLevel) : m_logLevel(defaultLevel) {
    m_logFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    m_logFile.open(filename, std::ios_base::app);
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}



void Logger::log(const std::string& message, LogLevel level) {
    Logger& logger = getInstance();
    std::lock_guard<std::mutex> lock(logger.m_mutex);

    if (level >= logger.m_logLevel) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm timeinfo = {};

#ifdef _MSC_VER 
        localtime_s(&timeinfo, &in_time_t);
#else
        timeinfo = *localtime(&in_time_t);
#endif

        std::stringstream ss;
        ss << std::put_time(&timeinfo, "%Y-%m-%d %X");

        logger.m_logFile << "[" << ss.str() << "]"
            << "[" << logger.levelToString(level) << "]: "
            << message << std::endl;
    }
}

void Logger::setLogLevel(LogLevel newLevel) {
    Logger& logger = getInstance();
    std::lock_guard<std::mutex> lock(logger.m_mutex);
    logger.m_logLevel = newLevel;
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
    case LogLevel::LOW:
        return "LOW";
    case LogLevel::MEDIUM:
        return "MEDIUM";
    case LogLevel::HIGH:
        return "HIGH";
    default:
        return "UNKNOWN";
    }
}