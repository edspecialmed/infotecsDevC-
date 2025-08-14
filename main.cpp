#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <sstream>
#include <atomic>


#include "Logger.h"




std::queue<std::pair<std::string, LogLevel>> g_messageQueue;


std::mutex g_queueMutex;


std::condition_variable g_condition;


std::atomic<bool> g_done = false;



std::optional<LogLevel> parseLogLevel(const std::string& str) {
    if (str == "LOW") return LogLevel::LOW;
    if (str == "MEDIUM") return LogLevel::MEDIUM;
    if (str == "HIGH") return LogLevel::HIGH;
    return std::nullopt;
}



void worker_function() {
    
    while (!g_done) {
        std::pair<std::string, LogLevel> message_to_log;

        { 
            std::unique_lock<std::mutex> lock(g_queueMutex);

           
            g_condition.wait(lock, [] { return !g_messageQueue.empty() || g_done; });

           
            if (g_done && g_messageQueue.empty()) {
                break;
            }

          
            message_to_log = g_messageQueue.front();
            g_messageQueue.pop();

        } 

   
        Logger::log(message_to_log.first, message_to_log.second);
        std::cout << "Worker: Logged message -> " << message_to_log.first << std::endl;
    }
    std::cout << "Worker: Shutting down." << std::endl;
}




int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <log_filename> <DEFAULT_LOG_LEVEL>" << std::endl;
        std::cerr << "Example: " << argv[0] << " app.log MEDIUM" << std::endl;
        std::cerr << "Available levels: LOW, MEDIUM, HIGH" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    auto defaultLevelOpt = parseLogLevel(argv[2]);

    if (!defaultLevelOpt) {
        std::cerr << "Error: Invalid default log level provided. Use LOW, MEDIUM, or HIGH." << std::endl;
        return 1;
    }
    LogLevel defaultLevel = *defaultLevelOpt;

    
    if (!Logger::init(filename, defaultLevel)) {
        std::cerr << "Error: Failed to initialize logger." << std::endl;
        return 1;
    }

    
    std::thread workerThread(worker_function);

    
    std::cout << "Logger Test Application is running." << std::endl;
    std::cout << "Enter messages in the format: [LEVEL] message" << std::endl;
    std::cout << "If LEVEL is omitted, the default level '" << argv[2] << "' will be used." << std::endl;
    std::cout << "Type 'exit' to quit." << std::endl;

    for (std::string line; std::getline(std::cin, line);) {
        if (line == "exit") {
            break;
        }

        std::stringstream ss_for_command(line);
        std::string first_word_cmd;
        ss_for_command >> first_word_cmd;

        if (first_word_cmd == "SET_LEVEL") {
            std::string newLevelStr;
            ss_for_command >> newLevelStr;
            auto newLevelOpt = parseLogLevel(newLevelStr);
            if (newLevelOpt) {
                Logger::setLogLevel(*newLevelOpt);
                std::cout << "Main: LEVEL CHANGED TO ->  " << newLevelStr << std::endl;
            } else {
                std::cerr << "Error: wrong level for SET_LEVEL command. Use LOW, MEDIUM, или HIGH." << std::endl;
            }
            continue; 
        }





        std::stringstream ss(line);
        std::string first_word;
        ss >> first_word;

        auto levelOpt = parseLogLevel(first_word);
        std::string message;
        LogLevel level;

        if (levelOpt) {
           
            level = *levelOpt;
            
            std::getline(ss, message);
            
            if (!message.empty() && message[0] == ' ') {
                message = message.substr(1);
            }
        }
        else {
            
            level = defaultLevel;
            message = line;
        }

      
        {
            std::lock_guard<std::mutex> lock(g_queueMutex);
            g_messageQueue.push({ message, level });
        }

       
        g_condition.notify_one();
    }

    
    std::cout << "Main: Exit command received. Shutting down..." << std::endl;

    
    g_done = true;
    
    g_condition.notify_one();


    workerThread.join();

    std::cout << "Application finished." << std::endl;

    return 0;
}
