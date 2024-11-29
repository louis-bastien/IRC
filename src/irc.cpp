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
    
    int epoll_fd = epoll_create1(0):
    struct epoll_event epev;
    epev.events = EPOLLIN;
    epev.fd  = serv_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &epev);

    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < n, i++) {
            if (event[i].data.fd == serv_fd) {
                int client_fd = accept(serv_fd, NULL, NULL);
                epev.events = EPOLLIN;
                epev.fd  = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epev);
            } 
            else if (events[i].events & EPOLLIN) {
                char buffer[BUFFER_SIZE];
                int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
                if (bytes_read < 0) {
                    std::cout << "Read error" << std::endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
                if (bytes_read == 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
                else {
                    std::cout << buffer << std::endl;
                }
            }
        }
    }
    close(serv_fd);
    return 0;
}