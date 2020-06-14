#include "function.h"
#include <unordered_map>

class Timer
{
public:
    Timer()
    {
        //从创建就开始进行轮询
        pthread_t pth;
        pthread_create(&pth, NULL, ask, &timeMap);
        sleep(1);
    }

    //有新的客户端连接上
    void add(const int &fd)
    {
        time_t t;
        time(&t);
        timeMap[fd] = t;
    }

    //客户端关闭
    void deleteFd(const int &fd)
    {
        timeMap.erase(fd);
    }

    //客户端发来数据
    void update(const int &fd)
    {
        time_t t;
        time(&t);
        timeMap[fd] = t;
    }

private:
    unordered_map<int, time_t> timeMap; //<用户名fd,上一次发送包的时间>
};
