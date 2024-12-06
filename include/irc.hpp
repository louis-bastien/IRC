#pragma once
#include <sstream>
#include "Logger.hpp"
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <map>
#include <cstdlib>

void check_args(int argc, char **argv, Logger& logger);

