#include "../include/irc.hpp"

int main() {
    
    
    int client_fd;
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1234);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        std::cout << "Failed to create socket" << std::endl;
    connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    std::string mes = "Hello!";
    send(client_fd, mes.c_str(), mes.length(), 0);
    close(client_fd);
    return 0;
}