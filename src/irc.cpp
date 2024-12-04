#include "irc.hpp"
#include "Server.hpp"
#include "Logger.hpp"

int main(int argc, char** argv) {
    Logger logger("log/irc.log");
    try {
        check_args(argc, argv);
        logger.log(INFO, "Arguments checked and validated");
        Server ircServer(std::atoi(argv[1]), argv[2], logger);

        ircServer.init();
        logger.log(INFO, "Server initialized");
        ircServer.start();
    }
    catch (std::exception &e) { 
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void check_args(int argc, char **argv) {
    if (argc != 3)
        throw std::invalid_argument("Wrong number of arguments");

    std::stringstream ss(argv[1]);
    int value;
    ss >> value;

    if (ss.fail() || !ss.eof())
        throw std::invalid_argument("Port is not a valid int");
    else if (value < 1024 || value < 65535)
        throw std::invalid_argument("choose port between 1024 and 65535");
    
    std::string password(argv[2]);

    if (password.size() < 5 || password.length() > 12)
        throw std::invalid_argument("Choose password between 5 and 12 characters");
}