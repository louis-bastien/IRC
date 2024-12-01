#pragma once 

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <map>
#include "user.hpp"
#define BUFFER_SIZE 1024
#define MAX_EVENTS 24

