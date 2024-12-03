#pragma once
#include <sstream>

int check_args(int argc, char **argv);


#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#define BUFFER_SIZE 1024
#define MAX_EVENTS 24


