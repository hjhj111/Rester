#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H
#include<list>
#include<unordered_map>
#include<mutex>
#include<thread>
#include <functional>
#include <memory>
#include<sys/epoll.h>

//#include"utls.h"
#include"connection.h"
//#include"resterserver.h"

using namespace std;

//class Connection;

using ConnectionFunc=function<void(shared_ptr<Connection>conn)>;

class ConnectionThread
{
public:
    ConnectionThread()
    {

    };
    ~ConnectionThread()
    {
        running_= false;
        connections_new_.clear();
        connections_removed_.clear();
        connections_.clear();
        connection_thread_.join();
    }

    ConnectionThread(int max_connection,GetCallBack on_get);
    void Init();
    void AddConnection(ConnectionPtr conn);
    void DeleteConnection(ConnectionPtr conn);

    thread connection_thread_;
    atomic<bool> running_{true};

    list<ConnectionPtr> connections_;
    mutex mutex_new_;
    list<ConnectionPtr> connections_new_;
    mutex mutex_removed_;
    list<ConnectionPtr> connections_removed_;
    mutex mutex_map_;
    unordered_map<int,ConnectionPtr> fds_connections_;
    //mutex mutex_connections;
    int max_connection_;

    int epoll_fd_;

    GetCallBack on_get_;
    CloseCallBack on_close_;

    vector<ConnectionFunc> connection_funcs;
};

#endif // CONNECTIONTHREAD_H
