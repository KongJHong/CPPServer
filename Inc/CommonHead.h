#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/mman.h>       //mmap和munmap函数，共享内存传输文件
#include <sys/sendfile.h>   //sendfile函数直接传输文件
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>
#include <signal.h>
#include <iostream>

#define DEFAULT_PORT 8080       //默认端口
#define MAX_EVENT_NUMBER 1024   //默认epoll最大事件数
#define DEFAULT_SERVER_STRING "My Server"
#define TIME_OUT 10
