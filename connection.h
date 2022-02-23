#ifndef RESTERCONNECTION_H
#define RESTERCONNECTION_H
//#include"connectionthread.h"
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

#include <sys/epoll.h>


#include <memory>

#include "utls.h"

class ResterServer;
class ConnectionThread;

enum class ConnectionState
{
    HANG,
    ADDED,
    OVER
};

class Connection: public enable_shared_from_this<Connection>
{
public:
    Connection()
    {
        //fd_id= get_id();
    }
    ~Connection()
    {

        printf("close fd\n");
        //exit(-4);
        close(connectioned_fd_);
        printf("close fd over\n");
    }
    shared_ptr<Connection> GetShare()
    {
        return shared_from_this();

    }

    Connection(ResterServer* server);
    Connection(const Connection& connection);
    Connection& operator=(const Connection& connection);
    void Init(ConnectionThread* thread);
    bool operator==(const Connection& other);
public:
    unsigned long ip_;          //client ip
    unsigned short int port_;   //client port
    int connectioned_fd_;                   //connect socket
    //long long fd_id;
    bool is_on_;                //network switch
    ResterServer* server_;
    ConnectionThread* thread_;
    epoll_event event_;
    ConnectionState state_;
};

using ConnectionPtr=std::shared_ptr<Connection>;

#endif // RESTERCONNECTION_H
