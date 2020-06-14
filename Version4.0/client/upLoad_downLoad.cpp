#include "function.h"

/***************************puts*********************************/
void *upLoad(void *arg)
{
    bool flag;
    LoadTask *task = (LoadTask *)arg;
    string username = task->username;
    int Dir = task->Dir;
    //子线程重新连接开启新的描述符
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("sockfd");
        pthread_exit(NULL);
    }
    struct sockaddr_in sockinfo;
    bzero(&sockinfo, sizeof(sockinfo));
    sockinfo.sin_addr.s_addr = inet_addr(IP);
    sockinfo.sin_port = htons(PORT);
    sockinfo.sin_family = AF_INET;
    int ret = connect(sockfd, (sockaddr *)&sockinfo, sizeof(sockinfo));
    if (-1 == ret)
    {
        perror("connect");
        pthread_exit(NULL);
    }

    //将指令传送到服务器
    Packet packet;
    string orders = task->orders;
    packet.dataLen = strlen(orders.c_str());
    memcpy(packet.buf, orders.c_str(), packet.dataLen);
    sendCycle(sockfd, &packet, 4 + packet.dataLen);
    //解析命令
    stringstream ss(orders);
    string order, name, order2; //order代表命令，name代表跟在name后面的文件名，order2代表补充命令（可为空）
    ss >> order >> name >> order2;
    const char *FILENAME = name.c_str();


    //发送用户姓名
    packet.dataLen = strlen(username.c_str());
    memcpy(packet.buf, username.c_str(), packet.dataLen);
    sendCycle(sockfd, &packet, 4 + packet.dataLen);
    //发送目录号
    sendCycle(sockfd, &Dir, 4);
    //发送MD5码
    string md5;
    ret = get_file_md5(name, md5);
    if (ret == -1)
    {
        perror("md5");
        pthread_exit(NULL);
    }
    packet.dataLen = strlen(md5.c_str());
    memcpy(packet.buf, md5.c_str(), packet.dataLen);
    sendCycle(sockfd, &packet, 4 + packet.dataLen);
    //1.判断文件是否重复
    recvCycle(sockfd, &flag, 1);
    if (!flag)
    {
        cout << "上传失败，文件名称相同！" << endl;
        pthread_exit(NULL);
    }
    //2.判断传送文件还是直接秒传
    recvCycle(sockfd, &flag, 1);
    if (flag)
    {
        cout << "100%! 秒传成功!" << endl;
    }
    else
    {
        cout << endl;
        sendFile(sockfd, FILENAME);
    }
    close(sockfd);
    pthread_exit(NULL);
}

int sendFile(int sockfd, const char *FILENAME)
{
    int ret;
    Packet packet;
    int file_fd = open(FILENAME, O_RDWR);
    ERROR_CHECK(file_fd, -1, "open");
    //传送文件大小
    struct stat statbuf;
    ret = stat(FILENAME, &statbuf);
    if (-1 == ret)
    {
        perror("stat");
        pthread_exit(NULL);
    }
    off_t filesize = statbuf.st_size;
    packet.dataLen = sizeof(off_t);
    memcpy(packet.buf, &filesize, packet.dataLen);
    sendCycle(sockfd, &packet, sizeof(int) + packet.dataLen);
    //传送文件内容
    char *pmap = (char *)mmap(NULL, filesize, PROT_READ, MAP_SHARED, file_fd, 0);
    ERROR_CHECK(pmap, (char *)-1, "mmap");
    off_t offset = 0, lastsize = 0;
    off_t slice = filesize / 100;
    while (1)
    {
        if (filesize > offset + (off_t)sizeof(packet.buf))
        {
            packet.dataLen = sizeof(packet.buf);
            memcpy(packet.buf, pmap + offset, packet.dataLen);
            sendCycle(sockfd, &packet, sizeof(int) + packet.dataLen);
            offset += packet.dataLen;
            //打印
            if (offset - lastsize > slice)
            {
                printf("\r%5.2f%%", (float)offset / filesize * 100);
                fflush(stdout);
                lastsize = offset;
            }
        }
        else
        {
            packet.dataLen = filesize - offset;
            memcpy(packet.buf, pmap + offset, packet.dataLen);
            sendCycle(sockfd, &packet, sizeof(int) + packet.dataLen);
            break;
        }
    }
    printf("\r100.0000000%%\n");
    ret = munmap(pmap, filesize);
    ERROR_CHECK(ret, -1, "munmap");
    //发送传送结束标志
    packet.dataLen = 0;
    sendCycle(sockfd, &packet, sizeof(int));

    close(file_fd);
    return 0;
}

/***************************gets*********************************/
void *downLoad(void *arg)
{
    bool flag;
    LoadTask *task = (LoadTask *)arg;
    string username = task->username;
    int Dir = task->Dir;
    //子线程重新连接开启新的描述符
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("sockfd");
        pthread_exit(NULL);
    }
    struct sockaddr_in sockinfo;
    bzero(&sockinfo, sizeof(sockinfo));
    sockinfo.sin_addr.s_addr = inet_addr(IP);
    sockinfo.sin_port = htons(PORT);
    sockinfo.sin_family = AF_INET;
    int ret = connect(sockfd, (sockaddr *)&sockinfo, sizeof(sockinfo));
    if (-1 == ret)
    {
        perror("connect");
        pthread_exit(NULL);
    }

    //将指令传送到服务器
    Packet packet;
    string orders = task->orders;
    packet.dataLen = strlen(orders.c_str());
    memcpy(packet.buf, orders.c_str(), packet.dataLen);
    sendCycle(sockfd, &packet, 4 + packet.dataLen);
    //解析命令
    stringstream ss(orders);
    string order, name, order2; //order代表命令，name代表跟在name后面的文件名，order2代表补充命令（可为空）
    ss >> order >> name >> order2;
    const char *FILENAME = name.c_str();
    off_t offset = stol(order2);

    //发送用户姓名
    packet.dataLen = strlen(username.c_str());
    memcpy(packet.buf, username.c_str(), packet.dataLen);
    sendCycle(sockfd, &packet, 4 + packet.dataLen);
    //发送目录号
    sendCycle(sockfd, &Dir, 4);

    //接收服务器的判断，当前目录下有无此文件
    recvCycle(sockfd, &flag, 1);
    if (!flag)
    {
        cout << "没有该文件!" << endl;
        pthread_exit(NULL);
    }
    //开始接收文件
    cout << "开始接收文件" << endl;
    recvFile(sockfd, FILENAME, offset);

    close(sockfd);
    pthread_exit(NULL);
}

//断点续传信息包
struct LoadPacket
{
    int fd;
    off_t loadsize;
} loadPacket;

void handler(int signum)
{
    cout << "终止下载" << endl;
    ftruncate(loadPacket.fd, loadPacket.loadsize); //断点续传存储点
    cout << "已下载" << loadPacket.loadsize  << "B" << endl;
    exit(-1);
}

int recvFile(int sockfd, const char *FILENAME, off_t offset)
{
    bzero(&loadPacket, sizeof(loadPacket));
    signal(SIGINT, handler);
    int ret;
    int dataLen;
    char buf[1000] = {0};

    //接收文件名称
    int file_fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
    ERROR_CHECK(file_fd, -1, "open");
    loadPacket.fd = file_fd;

    //接收文件大小
    recvCycle(sockfd, &dataLen, sizeof(int));
    recvCycle(sockfd, buf, dataLen);
    off_t filesize;
    memcpy(&filesize, buf, dataLen);
    ftruncate(file_fd, filesize);

    //接收文件内容
    char *pmap = (char *)mmap(NULL, filesize, PROT_WRITE, MAP_SHARED, file_fd, 0);
    ERROR_CHECK(pmap, (char *)-1, "mmap");
    off_t download = offset, lastsize = offset;
    off_t slice = filesize / 100;

    while (1)
    {
        recvCycle(sockfd, &dataLen, sizeof(int));
        if (0 == dataLen)
        {
            printf("\r100.00%%\n");
            break;
        }
        else
        {
            ret = recvCycle(sockfd, pmap + download, dataLen);
            if (ret == 0)
                break; //服务器已经断开
            if (ret == -1)
            {
                break;
            }
            download += dataLen;
            loadPacket.loadsize = download;
            if (download - lastsize > slice)
            {
                printf("\r%5.2f%%", (float)download / filesize * 100);
                fflush(stdout);
                lastsize = download;
            }
        }
    }
    ret = munmap(pmap, filesize);
    ERROR_CHECK(ret, -1, "munmap");

    close(file_fd);
    return 0;
}