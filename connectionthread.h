#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H
#include<list>
#include<unordered_map>
#include<mutex>
#include<sys/epoll.h>

#include"utls.h"
#include"connection.h"
#include"resterserver.h"

using namespace std;

//class Connection;



class ConnectionThread
{
public:
    ConnectionThread()
    {

    };
    ConnectionThread(int max_connection,GetCallBack on_get);
    void Init();
    void AddConnection(ConnectionPtr conn);
    void DeleteConnection(ConnectionPtr conn);

    list<ConnectionPtr> connections_;
    list<ConnectionPtr> connections_new_;
    list<ConnectionPtr> connections_removed_;
    unordered_map<int,ConnectionPtr> fds_connections_;
    mutex mutex_connections;
    int max_connection_;

    int epoll_fd_;

    GetCallBack on_get_;
    CloseCallBack on_close_;
};

#endif // CONNECTIONTHREAD_H
