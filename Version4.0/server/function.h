#pragma once
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

#define ARGS_CHECK(argc, val)       \
    {                               \
        if (argc != val)            \
        {                           \
            printf("Args ERROR\n"); \
            return -1;              \
        }                           \
    }
#define THREAD_ERROR_CHECK(ret, funcName)               \
    {                                                   \
        if (ret != 0)                                   \
        {                                               \
            printf("%s:%s\n", funcName, strerror(ret)); \
            return -1;                                  \
        }                                               \
    }
#define ERROR_CHECK(ret, retval, filename) \
    {                                      \
        if (ret == retval)                 \
        {                                  \
            perror(filename);              \
            return -1;                     \
        }                                  \
    }

/*****************************************结构体定义部分******************************************/
#define IP "172.21.0.7"
#define PORT 2000
#define THREADNUM 10
#define MAXEVENTS 10

struct Packet
{
    int dataLen;    // 包头，用来记录存数据的大小
    char buf[1000]; // 包数据，用来存放数据
};


//创建TCP通信socket
int tcpInit(int *);

//接收,发送文件
int recvFile(int sockfd);
int sendFile(int sockfd, const char *);

//循环接收，发送数据
int recvCycle(int, void *, size_t);
int sendCycle(int, void *, size_t);

//产生随机字符串（此处用于产生Salt）
string GenerateStr(int STR_LEN);

//轮询函数
void *ask(void *arg);

