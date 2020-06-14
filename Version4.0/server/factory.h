#pragma once
#include "function.h"
#include "workQue.h"
#include "MyDb.h"
#include "MyLog.h"

void *doingTask(void *arg);
int childHandle(const Task &task);
class Factory
{
public:
    Factory()
    {
        threadNum = THREADNUM;
        cond = PTHREAD_COND_INITIALIZER;
        startFlag = false;
    }
    void startFactory()
    {
        if (!startFlag)
        {
            cout << "server is running ..." << endl;
            for (int i = 0; i < threadNum; i++)
            {
                pthread_create(&threads[i], NULL, doingTask, this);
            }
            sleep(1);
            startFlag = true;
        }
    }

    WorkQue que;
    int threadNum;
    pthread_t threads[THREADNUM];
    pthread_cond_t cond;
    bool startFlag;
};

void *doingTask(void *arg)
{
    Factory *f = (Factory *)arg;
    while (1)
    {
        pthread_mutex_lock(&f->que.mutex);
        while (0 == f->que.size())
            pthread_cond_wait(&f->cond, &f->que.mutex);
        Task task = f->que.getTask();
        childHandle(task);
        pthread_mutex_unlock(&f->que.mutex);
    }
}

//子线程只负责处理puts和gets这两个比较耗时的命令
int childHandle(const Task &task)
{
    MyDb db;
    db.initDB("localhost", "root", "123", "Netdisk");
    int dataLen;
    char buf[1000] = {0};
    int ret;
    bool flag;
    string sql;
    string res;
    off_t filesize;
    //分析任务包的命令
    int sockfd = task.fd;
    stringstream ss(task.orders);
    cout << "ChildHandle:" << task.orders << endl;
    string order, name, order2; //order代表命令，name代表跟在name后面的文件名，order2代表补充命令（可为空）
    ss >> order >> name >> order2;
    string filename(name);
    const char *FILENAME = filename.c_str();

    string username = task.username;
    cout << "username:" << username << endl;

    int Dir = task.Dir;
    cout << "Dir:" << Dir << endl;
    LOG(username, task.orders);
    /*******************************puts 文件内容*******************************/
    if ("puts" == order)
    {
        //接收MD5码
        bzero(buf, sizeof(buf));
        recvCycle(sockfd, &dataLen, 4);
        recvCycle(sockfd, buf, dataLen);
        string md5(buf);
        cout << "md5:" << md5 << endl;
        /**puts前需要进行两点判断：
         * 1.在当前用户，当前目录下是否拥有此文件
         * 2.虚拟目录中已经存在相同的文件，可以直接秒传
        */
        //1.判断文件是否重复
        sql = "SELECT FileName FROM Virtual_Dir WHERE User = '" + username + "' AND Dir = " + to_string(Dir) + " AND FileType = 'f' AND FileName = '" + filename + "'";
        db.select_one_SQL(sql, res);
        if (res.empty())
        {
            flag = true;
            sendCycle(sockfd, &flag, 1);
        }
        else
        {
            flag = false;
            sendCycle(sockfd, &flag, 1);
            return 0;
        }

        //2.判断是否使用秒传
        sql = "SELECT MD5 FROM Virtual_Dir WHERE MD5 = '" + md5 + "'";
        db.select_one_SQL(sql, res);
        cout << sql << endl;
        cout << res << endl;
        if (!res.empty())
        {
            cout << "秒传" << endl;
            flag = true;
            sendCycle(sockfd, &flag, 1);
        }
        else
        {
            flag = false;
            sendCycle(sockfd, &flag, 1);
            int file_fd = open(md5.c_str(), O_RDWR | O_CREAT, 0666);
            ERROR_CHECK(file_fd, -1, "open");
            //接收文件大小
            bzero(buf, sizeof(buf));
            recvCycle(sockfd, &dataLen, sizeof(int));
            recvCycle(sockfd, buf, dataLen);
            memcpy(&filesize, buf, dataLen);
            ftruncate(file_fd, filesize);
            //接收文件内容
            bzero(buf, sizeof(buf));
            char *pmap = (char *)mmap(NULL, filesize, PROT_WRITE, MAP_SHARED, file_fd, 0);
            ERROR_CHECK(pmap, (char *)-1, "mmap");
            off_t download = 0, lastsize = 0;
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
                    download += dataLen;
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
        }
        sql = "INSERT INTO Virtual_Dir(Dir, FileName, FileType, MD5, User) VALUES(" + to_string(Dir) + ",'" + filename + "' , 'f' , '" + md5 + "', '" + username + "')";
        db.exeSQL(sql);
    }
    /*******************************gets 文件内容*******************************/
    else if ("gets" == order)
    {
        //先判断当前目录下有无该文件
        sql = "SELECT FileName FROM Virtual_Dir WHERE User = '" + username + "' AND Dir = " + to_string(Dir) + " AND FileType = 'f' AND FileName = '" + filename + "'";
        db.select_one_SQL(sql, res);
        if (!res.empty())
        {
            flag = true;
            sendCycle(sockfd, &flag, 1);
        }
        else
        {
            flag = false;
            sendCycle(sockfd, &flag, 1);
            return 0;
        }

        //在虚拟文件目录中实际文件名称的存储是使用MD5码作为名字的
        sql = "SELECT MD5 FROM Virtual_Dir WHERE User = '" + username + "' AND Dir = " + to_string(Dir) + " AND FileType = 'f' AND FileName = '" + filename + "'";
        db.select_one_SQL(sql, res);
        FILENAME = res.c_str();

        //判断文件的偏移（断点续传功能）
        off_t beginPos = stol(order2);
        cout << "beginPos = " << beginPos << endl;

        //开始发送文件
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
        off_t offset = beginPos, lastsize = beginPos;
        off_t slice = filesize / 100;
        while (1)
        {
            if (filesize > offset + (off_t)sizeof(packet.buf))
            {
                packet.dataLen = sizeof(packet.buf);
                memcpy(packet.buf, pmap + offset, packet.dataLen);
                ret = sendCycle(sockfd, &packet, sizeof(int) + packet.dataLen);
                if (-1 == ret)
                {
                    close(file_fd);
                    return -1;
                }
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
                ret = sendCycle(sockfd, &packet, sizeof(int) + packet.dataLen);
                if (-1 == ret)
                {
                    close(file_fd);
                    return -1;
                }
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
    }
    return 0;
}