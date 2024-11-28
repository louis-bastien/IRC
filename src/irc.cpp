#include "irc.hpp"

int main(int argc, char** argv) {
    
    if (argc != 3) {
        std::cout << "Wrong number of arguments. Try the following:" << std::endl;
        std::cout << "./ircserv 4242 mypassword" << std::endl;
        return 1;
    }

    std::string port(argv[1]);
    if (port.find_first_not_of("0123456789") != std::string::npos) {
        std::cout << "Port wrong: use only digits!" << std::endl;
        return 1;
    }

    return 0;
}