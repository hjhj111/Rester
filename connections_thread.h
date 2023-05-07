#ifndef CONNECTIONTHREAD_H
#define CONNECTIONTHREAD_H
#include<list>
#include<unordered_map>
#include<mutex>
#include<thread>
#include <functional>
#include <memory>
#include<sys/epoll.h>

#include"connection.h"

using namespace std;

using ConnectionFunc=function<void(shared_ptr<Connection>conn)>;

class ConnectionsThread
{
public:
    ConnectionsThread(const ConnectionsThread& ) = delete;

    ConnectionsThread& operator = (const ConnectionsThread& ) = delete;

    ConnectionsThread(Rester* server, int max_connection);

    ~ConnectionsThread()
    {
        running_= false;
        connections_new_.clear();
        //connections_removed_.clear();
        //connections_.clear();
        connection_thread_.join();
    }

    void Init();

    void AddConnection(ConnectionPtr conn);

    void DeleteConnection(ConnectionPtr conn);

    Rester* server_;
    thread connection_thread_;
    atomic<bool> running_{true};
    int max_connection_;
    int epoll_fd_;

    mutex mutex_new_;
    list<ConnectionPtr> connections_new_;
    unordered_map<int,ConnectionPtr> fds_connections_;
//    mutex mutex_removed_;
//    list<ConnectionPtr> connections_removed_;

private:
    int AddConnectionTrue(ConnectionPtr conn);
};

#endif // CONNECTIONTHREAD_H
