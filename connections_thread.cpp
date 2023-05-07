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
#include "connections_thread.h"
#include "connection.h"

#include <algorithm>

#include "rester.h"

ConnectionsThread::ConnectionsThread(Rester* server, int max_connection):
    max_connection_(max_connection),
    server_(server)
{

}

void ConnectionsThread::Init()
{
    epoll_fd_ = epoll_create(max_connection_);

    auto thread_fun=[&]()
    {
        int ret=-1;
        while (running_)
        {
            //printf("connection thread running\n");
            //add new connection from main event loop
            list<ConnectionPtr> connections_new;
            {
                lock_guard<mutex> guard(mutex_new_);
                if(!connections_new_.empty())
                {
                    connections_new.swap(connections_new_);
                    connections_new_.clear();
                }
            }
            for(auto conn:connections_new)
            {
                AddConnectionTrue(conn);
            }
            epoll_event events[max_connection_];
            memset(events, 0x00, sizeof(epoll_event)*max_connection_);

            int n_fd = epoll_wait(epoll_fd_, events, max_connection_, 5);
//            if(fds_connections_.size()>1)
//            {
//                printf("connection nums   %d\n",fds_connections_.size());
//            }
            if(n_fd <0 && errno == EINTR)
            {
                //exit(3);
                continue;
            }
            for(int i = 0; i < n_fd; ++i)
            {
                int event_size=0;
                if(events[i].events & EPOLLIN)//read,client close or down
                {
                    event_size++;
//                    if(fds_connections_.find(events[i].data.fd)!=fds_connections_.end())
//                    {
                    auto conn = fds_connections_.at(events[i].data.fd);
                    server_->on_read_(conn);
//                    }
                }
                if(events[i].events & EPOLLOUT)  //connected also trigger
                {
                    event_size++;
//                    if(fds_connections_.count(events[i].data.fd))
//                    {
                        auto conn=fds_connections_.at(events[i].data.fd);
                        server_->on_write_(conn);
//                    }
//                    else
//                    {
//                      exit(-333);
//                    }

                }
                if(events[i].events & EPOLLRDHUP )//peer close can trigger except for epollin
                {
                    event_size++;
                    perror("epoll rdhup || err");
                    if(fds_connections_.count(events[i].data.fd))
                    {
                        auto conn = fds_connections_.at(events[i].data.fd);
                        conn->Close();
//                        printf("conn ptr count: %d",conn.use_count());
//                        exit(8);
                    }
                }
                if(events[i].events & EPOLLERR)
                {
                    if(fds_connections_.count(events[i].data.fd))
                    {
                        auto conn = fds_connections_.at(events[i].data.fd);
                        conn->Close();
//                        printf("conn ptr count: %d",conn.use_count());
//                        exit(8);
                    }
                }
                printf("event_size: %d\n",event_size);
            }

        }
    };
    connection_thread_=thread(thread_fun);
}

void ConnectionsThread::AddConnection(ConnectionPtr conn)
{
    {
        lock_guard<mutex> guard(mutex_new_);
        connections_new_.push_back(conn);
    }
}

void ConnectionsThread::DeleteConnection(ConnectionPtr conn)
{
    //printf(" delete\n");
    int ret=epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, conn->connected_fd_ , &conn->event_);
    if(ret<0)
    {
        perror("epoll remove wrong\n");
        exit(12);
    }
//    if(fds_connections_.count(conn->connected_fd_))
//    {
        fds_connections_.erase(conn->connected_fd_);
//    }
}

int ConnectionsThread::AddConnectionTrue(ConnectionPtr conn)
{
    int ret=0;
    int flags = fcntl(conn->connected_fd_, F_GETFL, 0);
    if(flags < 0)
    {
        perror("fcntl F_GETFL");
        return ret;
        exit(2);
    }
    flags |= O_NONBLOCK;//设置成非阻塞模式
    ret = fcntl(conn->connected_fd_, F_SETFL, flags);
    if(ret < 0)
    {
        perror("fcntl F_SETFL");
        return ret;
        exit(2);
    }
    ret=epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, conn->connected_fd_ , &conn->event_);
    if(ret<0)
    {
        perror("epoll add wrong\n");
        return ret;
        exit(2);
    }
    fds_connections_[conn->connected_fd_]=conn;
}

