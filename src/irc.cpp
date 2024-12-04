#include "irc.hpp"
#include "server.hpp"


int main(int argc, char** argv) {
    try {
        check_args(argc, argv);
        Server ircServer(atoi(argv[1]), argv[2]);
        ircServer.initServer();
        ircServer.startServer();
    }
    catch (std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int check_args(int argc, char **argv) {
    if (argc != 3)
        throw std::invalid_argument::("Wrong number of arguments");

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


    int serv_fd;
    sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1234);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd < 0) {
        std::cout << "Failed to create socket" << std::endl;
        return 1;
    }
    if (bind(serv_fd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Bind failed" << std::endl;
            return 1;
    }

    listen(serv_fd, 5);
    
    int epoll_fd = epoll_create1(0);
    struct epoll_event epev;
    epev.events = EPOLLIN;
    epev.data.fd  = serv_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &epev);

    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == serv_fd) {
                int client_fd = accept(serv_fd, NULL, NULL);
                epev.events = EPOLLIN;
                epev.data.fd  = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epev);
            } 
            else if (events[i].events & EPOLLIN) {
                char buffer[BUFFER_SIZE];
                int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
                if (bytes_read < 0) {
                    std::cout << "Read error" << std::endl;
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
                if (bytes_read == 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
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