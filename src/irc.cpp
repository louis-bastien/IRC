#include "irc.hpp"
#include "Server.hpp"

int main(int argc, char** argv) {
    Logger logger("log/irc.log");
    logger.log(INFO, "####STARTED LOGGER####");
    try {
        check_args(argc, argv, logger);
        logger.log(INFO, "Arguments checked and validated");
        Server ircServer(std::atoi(argv[1]), argv[2], logger);

        ircServer.init();
        logger.log(INFO, "Server started successfully");
        ircServer.start();
    }
    catch (std::exception &e) { 
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void check_args(int argc, char **argv, Logger& logger) {
    if (argc != 3) {
        logger.log(ERROR, "Wrong number of arguments (" + Utils::toString(argc) + ")");
        throw std::invalid_argument("Wrong number of arguments.");
    }
    
    std::stringstream ss(argv[1]);
    int value;
    ss >> value;

    if (ss.fail() || !ss.eof())
        throw std::invalid_argument("Port is not a valid int");
    else if (value < 1024 || value > 65535) {
        logger.log(ERROR, "Wrong port selected (" + Utils::toString(value) + ")");
        throw std::invalid_argument("choose port between 1024 and 65535");
    }
    
    std::string password(argv[2]);

    if (password.size() < 5 || password.length() > 12) {
        logger.log(ERROR, "Wrong password selected (" + password + ")");
        throw std::invalid_argument("Choose password between 5 and 12 characters");
    }
}