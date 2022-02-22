#ifndef RESTERSERVER_H
#define RESTERSERVER_H
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include <sys/epoll.h>

#include<functional>
#include <list>

#include "http-parser/http_parser.h"
#include "config.h"
#include"threadpool.h"

using namespace std;

//class Connection;


//class ThreadPool;

class ResterServer
{
public:
    ResterServer(const Config&);
    bool Init();

    GetCallBack on_get;
    PostCallBack on_post;
    ThreadPool thread_pool_;
public:
    int listen_fd_;
    int epoll_fd_;
    int max_connection_;
    int max_thread_;
private:
    ConnectCallBack on_connected;
    list<shared_ptr<Connection>> connections_;

};

#endif // RESTERSERVER_H
