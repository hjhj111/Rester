#ifndef RESTERCONNECTION_H
#define RESTERCONNECTION_H

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
//#include "connectionthread.h"

class Rester;

class ConnectionsThread;

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
        response_ptr_= make_shared<Response>();
    }

    Connection(const Connection& conn) = delete;

    Connection& operator=(const Connection& conn) = delete;

    ~Connection()
    {

        printf("close fd\n");
        close(connected_fd_);
//        if(buf_size_>0)
//        {
//            delete buf_;
//        }
        printf("close fd over\n");
    }

    shared_ptr<Connection> GetShare()
    {
        return shared_from_this();
    }



    Connection(Rester* server);

    void Init(ConnectionsThread* thread);

    bool operator==(const Connection& other);

    void Close();

    void SetOnWrite(GetCallBack on_write);

    void OnConnect()
    {
        if(on_connect_)
        {
            on_connect_(shared_from_this());
        }
    }

    void OnClose()
    {
        if(on_close_)
        {
            on_close_(shared_from_this());
        }
    }

//convenient to operate
public:
    unsigned int ip_;          //client ip
    unsigned short int port_;   //client port
    int connected_fd_;          //connect socket
    bool is_on_;                //network switch
    Rester* server_;
    ConnectionsThread* thread_;
    epoll_event event_;
    ConnectionState state_;

    int sent_size_=0;
    bool read=false;
    //callback
    //PostCallBack  on_post_;
    //GetCallBack  on_get_;
    GetCallBack on_write_;// for all methods
    CloseCallBack on_close_;

    //deprecated
    ConnectCallBack  on_connect_;

    RequestPtr request_ptr_;
    ResponsePtr response_ptr_;
};

using ConnectionPtr=std::shared_ptr<Connection>;

#endif // RESTERCONNECTION_H
