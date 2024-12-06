#include "../include/irc.hpp"


//Simple client tester. to execute : ./executable [port] [message]
int main(int argc, char **argv) {   
    int client_fd;
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(std::atoi(argv[1]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        std::cout << "Failed to create socket" << std::endl;
    connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    std::string message(argv[2]);
    message += "\r\n";
    send(client_fd, message.c_str(), message.length(), 0);
    close(client_fd);
    return 0;
}