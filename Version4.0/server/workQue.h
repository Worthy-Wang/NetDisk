#pragma once
#include "function.h"
class Task
{
public:
    Task(int fd, string orders, string username, int Dir)
    :fd(fd), orders(orders), username(username), Dir(Dir)
    {
    }
    
    int fd; //客户端socket
    string orders; //客户端发送来的命令
    string username; //客户端登陆用户名
    int Dir; //用户所在目录号
};

class WorkQue
{
public:
    WorkQue()
    {
        mutex = PTHREAD_MUTEX_INITIALIZER;
    }
    void insertTask(const Task& task) //向任务队列中插入新的任务
    {
        deq.push_back(task);
    }
    int size()
    {
        return deq.size();
    }
    Task getTask() //线程取得新的任务
    {
        Task ans = deq.front();
        deq.pop_front();
        return ans;
    }
    pthread_mutex_t mutex; //多线程对任务队列的互斥锁
    deque<Task> deq; //存放accept 接收的 fd，
};
