#ifndef RESTERSERVER_H
#define RESTERSERVER_H
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<cstdio>
#include<cstdlib>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include <sys/epoll.h>

#include<functional>
#include <list>

#include "threads_pool.h"
#include "router.h"


class Rester
{

public:
    Rester(const Config&);

    Rester(const Rester& ) = delete;

    Rester& operator = (const Rester& ) = delete;

    void Init();

    void AddWorker(const Router& worker);

    void PrintWorkers()
    {
        for(auto& worker:url_workers_)
        {
            printf("worker: %s is running\n",worker.first.c_str());
        }
    }

public:
    ThreadPool* thread_pool_;
    //ThreadPool a;
    std::atomic<bool> running_{true};
    int server_port_;
    int listen_fd_;
    int epoll_fd_;
    int max_connection_;
    int max_thread_;

    ReadCallBack on_read_;
    ConnectCallBack  on_connect_;
    WriteCallBack  on_write_;
private:
    list<shared_ptr<Connection>> connections_;
    unordered_map<string,Router> url_workers_;
};

#endif // RESTERSERVER_H
