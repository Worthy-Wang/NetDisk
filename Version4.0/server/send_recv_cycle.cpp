#include "function.h"
int recvCycle(int fd, void *p, size_t len)
{
    char *pStart = (char *)p;
    size_t size = 0;
    while (size < len)
    {
        int ret = recv(fd, pStart + size, len - size, 0);
        if (0 == ret)
        {
            return 0;
        }
        ERROR_CHECK(ret, -1, "recv");
        size += ret;
    }
    return len;
}

int sendCycle(int fd, void *p, size_t len)
{
    char *pStart = (char *)p;
    size_t size = 0;
    while (size < len)
    {
        int ret = send(fd, pStart + size, len - size, 0);
        if (-1 == ret )
            return -1;
        size += ret;
    }
    return len;
}
