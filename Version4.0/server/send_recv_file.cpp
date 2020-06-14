#include "function.h"

int recvFile(int sockfd)
{
    // int ret;
    // int dataLen;
    // char buf[1000] = {0};




    return 0;
}

int sendFile(int client_fd, const char *FILENAME)
{
    int ret;
    Packet packet;
    int file_fd = open(FILENAME, O_RDWR);
    ERROR_CHECK(file_fd, -1, "open");

    //传送文件名称
    packet.dataLen = strlen(FILENAME);
    strcpy(packet.buf, FILENAME);
    sendCycle(client_fd, &packet, sizeof(int) + packet.dataLen);

    //传送文件大小
    struct stat statbuf;
    ret = stat(FILENAME, &statbuf);
    ERROR_CHECK(ret, -1, "stat");
    packet.dataLen = sizeof(off_t);
    memcpy(packet.buf, &statbuf.st_size, packet.dataLen);
    sendCycle(client_fd, &packet, sizeof(int) + packet.dataLen);

    //传送文件内容
    char *pmap = (char *)mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, file_fd, 0);
    ERROR_CHECK(pmap, (char *)-1, "mmap");
    off_t offset = 0, lastsize = 0;
    off_t slice = statbuf.st_size / 100;
    while (1)
    {
        if (statbuf.st_size > offset + (off_t)sizeof(packet.buf))
        {
            packet.dataLen = sizeof(packet.buf);
            memcpy(packet.buf, pmap + offset, packet.dataLen);
            sendCycle(client_fd, &packet, sizeof(int) + packet.dataLen);
            offset += packet.dataLen;
            //打印
            if (offset - lastsize > slice)
            {
                printf("\r%5.2f%%", (float)offset / statbuf.st_size * 100);
                fflush(stdout);
                lastsize = offset;
            }
        }
        else
        {
            packet.dataLen = statbuf.st_size - offset;
            memcpy(packet.buf, pmap + offset, packet.dataLen);
            sendCycle(client_fd, &packet, sizeof(int) + packet.dataLen);
            break;
        }
    }
    printf("\r100.00%%\n");
    ret = munmap(pmap, statbuf.st_size);
    ERROR_CHECK(ret, -1, "munmap");
    //发送传送结束标志
    packet.dataLen = 0;
    sendCycle(client_fd, &packet, sizeof(int));

    close(file_fd);
    return 0;
}
