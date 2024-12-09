#include "Logger.hpp"

Logger::Logger(const std::string& filepath) {
    _logFile.open(filepath.c_str(), std::ofstream::app | std::fstream::out);
    if (!_logFile.is_open()) {
        throw std::runtime_error("Failed to open log file");
    }
}

Logger::~Logger() {
    if(_logFile.is_open())
        _logFile.close();
}

void Logger::log(logLevel level, const std::string& message) {
    //Get the current time in following format YYY-MM-DD HH:MM:SS
    time_t now = time(0);
    tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    //Write to the logfile if available
    if(_logFile.is_open())
        _logFile << "[" << timestamp << "] " << levelToStr(level) << " " << message << std::endl;
    else
        std::cerr << "Cannot write to logfile" << std::endl;
}

std::string Logger::levelToStr(logLevel level) {
    switch(level) {
        case DEBUG:
            return("DEBUG");
        case INFO:
            return("INFO");
        case WARNING:
            return("WARNING");
        case ERROR:
            return("ERROR");
        default:
            return("UNKNOWN");
    }
}