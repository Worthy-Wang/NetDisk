# NetDisk
### FTP文件服务器设计思路：
#### 1.整体架构采用线程池结合epoll监听
客户端通过sockfd与服务器通信，服务器通过sockfd进行连接新的客户端，用accept到的newfd与客户端进行通信；
主线程负责向子线程分配任务，子线程负责处理任务并于客户端通信；
简单命令主线程直接执行，puts 和 gets 命令交给子线程执行。

#### 2.文件系统采用虚拟文件目录设计，将所有的文件存放在一个公共目录中，自己只能看到自己的文件，并创建好一个存放文件信息的数据库（此处数据库采用MYSQL）
FileId | Dir | FileName | FileSize |FileType | MD5  |User | 
FileId : 文件ID， 主键, 自增型
Dir : 所在目录，0则表示根目录
FileName : 文件名称，可以相同，区分靠FileId
FileSize : 文件大小(B显示)
FileType : f 代表文件类型，d代表文件夹类型
MD5 : 文件的MD5值，用来判断文件的内容是否相同
User ：文件的所属对象



### 命令实现：
1.cd  进入对应目录
2.ls 列出相应目录文件
3.puts 将本地文件上传至服务器
4.gets 文件名 下载服务器文件到本地
5.mkdir 增加文件夹
6.rm 删除服务器上文件
7.quit 退出
8.其他命令不响应


### 功能实现：
1.用户登陆与密码验证
类似/etc/shadow文件下的盐值加密，使用crypt函数进行加密，将用户账号，密码以及加密后的数据存放在数据库中；
盐值salt(随机生成)的8个字符，密码匹配方式是服务器根据客户端的用户名找到其Salt，并将Salt发送回客户端，客户端进行crypt加密后再发送给服务器进行匹配，其中需要注意客户端是没有权限访问数据库的
User | Password | Salt | Cipher

2.记录日志信息：包括客户端的请求信息，客户端连接时间，客户端的操作记录以及操作时间，并将信息都存放在数据库中；
User | Operation | time

3.文件的断点续传：客户端gets过程中如果断开，再次gets时，从断点开始传输
具体实现：客户端如果有要接收的文件file并已经下载了1000字节，则向服务器发送 gets file 1000, 服务器直接从偏移1000字节的位置开始传送，客户端也偏移1000字节开始接收

4.使用token令牌
token的介绍：https://www.jianshu.com/p/24825a2683e6

5.连接上的客户端如果30s没有相应，那么关闭描述符

https://mp.weixin.qq.com/s?__biz=MjM5ODYxMDA5OQ==&mid=2651959957&idx=1&sn=a82bb7e8203b20b2a0cb5fc95b7936a5&chksm=bd2d07498a5a8e5f9f8e7b5aeaa5bd8585a0ee4bf470956e7fd0a2b36d132eb46553265f4eaf&mpshare=1&scene=23&srcid=0718Qlp4AVKnZq1E1f144pE6#rd

此处采用轮询扫描法，设置Map<string,int> //<用户名,上一次发送包的时间>
设置一个timer,每秒都轮询Map中的元素，并将时间差大于30s的客户端连接断开 

6.多点下载（选做）



### 数据库创建（只需进入数据库执行一次即可）：

##########################################
### 创建数据库 Netdisk
##########################################
CREATE DATABASE Netdisk;
use Netdisk

##########################################
### 创建表 Virtual_Dir
##########################################
CREATE TABLE Virtual_Dir
(
  FileId    int             NOT NULL AUTO_INCREMENT,
  Dir       int             NOT NULL, 
  FileName varchar(10)      NOT NULL,
  FileSize BIGINT             NULL,
  FileType char(1)          NOT NULL,
  MD5      varchar(50)      NULL,
  User     varchar(10)      NOT NULL,
  PRIMARY KEY(FileId)
) ENGINE=InnoDB;

INSERT INTO Virtual_Dir(Dir, FileName, FileSize, FileType, MD5, User)
VALUES(0, 'file5', 512, 'f', 'md5', 'wwx');

##########################################
### 创建表 Shadow
##########################################
CREATE TABLE Shadow
(
    User    varchar(10)     NOT NULL,
    Password    varchar(18)     NOT NULL,
    Salt    char(8)     NOT NULL,
    Cipher  text        NOT NULL,
    PRIMARY KEY(User)
) ENGINE=InnoDB;

INSERT INTO Shadow
VALUES('wwx', '123', 'xxxxxxxx', 'xxx..xxx');

##########################################
### 创建表 Log
##########################################
CREATE TABLE Log
(
    Id     int   NOT NULL AUTO_INCREMENT,
    User    varchar(100)     NOT NULL,
    Operation    varchar(100)     NOT NULL,
    time         varchar(100)    NOT NULL,
    PRIMARY KEY(Id)
) ENGINE=InnoDB;

INSERT INTO Log (User, Operation, time)
VALUES();
