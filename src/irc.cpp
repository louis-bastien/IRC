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

    int serv_fd;
    sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1234);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd < 0)
        std::cout << "Failed to create socket" << std::endl;
    if (bind(serv_fd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Bind failed" << std::endl;
            return 1;
    }
    listen(serv_fd, 5);
    int client_fd = accept(serv_fd, NULL, NULL);
    std::cout << "Client fd = " << client_fd << std::endl;
    char buffer[1024] = {0};
    recv(client_fd, buffer, sizeof(buffer), 0);
    std::cout << "Message from client: " << buffer << std::endl;
    close(serv_fd);
    close(client_fd);
    return 0;
}