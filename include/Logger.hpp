#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

enum logLevel {DEBUG, INFO, WARNING, ERROR};

class Logger {
    private:
        std::ofstream _logFile;

        std::string levelToStr(logLevel level);
    public:
        Logger(const std::string& filepath);
        ~Logger();

        void log(logLevel level, const std::string& message);
};