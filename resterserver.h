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

//#include "http-parser/http_parser.h"

//#include "utls.h"
#include "threadpool.h"
#include "UrlWorker.h"
//class UrlWorker;
using namespace std;

class ResterServer
{

public:
    ResterServer(const Config&);

    void Init();

    void AddWorker(const UrlWorker& worker);

public:
    ThreadPool* thread_pool_;
    //ThreadPool a;
    atomic<bool> running_{true};
    int listen_fd_;
    int epoll_fd_;
    int max_connection_;
    int max_thread_;

    ReadCallBack on_read_;
    ConnectCallBack  on_connect_;
    WriteCallBack  on_write_;
private:
    list<shared_ptr<Connection>> connections_;
    unordered_map<string,UrlWorker> url_workers_;
};

#endif // RESTERSERVER_H
