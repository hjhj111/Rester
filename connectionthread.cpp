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
#include "connectionthread.h"
#include "connection.h"

#include <algorithm>

#include "resterserver.h"

ConnectionThread::ConnectionThread(ResterServer* server, int max_connection):
    max_connection_(max_connection),
    server_(server)
{

}

void ConnectionThread::Init()
{
    epoll_fd_ = epoll_create(max_connection_);

    auto thread_fun=[&]()
    {
        int ret=-1;
        while (running_)
        {
            //printf("connection thread running\n");
            //delete closed connection
            list<ConnectionPtr> connections_removed;
            {
                lock_guard<mutex> guard(mutex_removed_);
                if (!connections_removed_.empty())
                {
                    connections_removed.swap(connections_removed_);
                    connections_removed_.clear();
                }
            }
            for(auto conn:connections_removed)
            {
                ret=epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, conn->connected_fd_ , &conn->event_);
                if(ret<0)
                {
                    perror("epoll remove wrong\n");
                    continue;
                    exit(12);
                }
                connections_.remove(conn);
            }
            //add new connection
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
                /*设置成非阻塞模式*/
                int flags = fcntl(conn->connected_fd_, F_GETFL, 0);
                if(flags < 0)
                {
                      perror("fcntl F_GETFL");
                      exit(2);
                }
                flags |= O_NONBLOCK;
                ret = fcntl(conn->connected_fd_, F_SETFL, flags);
                if(ret < 0)
                {
                      perror("fcntl F_SETFL");
                      exit(2);
                }
                ret=epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, conn->connected_fd_ , &conn->event_);
                if(ret<0)
                {
                    perror("epoll add wrong\n");
                    exit(2);
                }
                connections_.push_back(conn);
            }
            epoll_event events[max_connection_];
            memset(events, 0x00, sizeof(epoll_event)*max_connection_);

            int n_fd = epoll_wait(epoll_fd_, events, max_connection_, 5);
//            if(fds_connections_.size()>1)
//            {
//                printf("connection nums   %d\n",fds_connections_.size());
//            }
            if(n_fd == -1 )//&& errno == EINTR
            {
                exit(3);
                continue;
            }
            for(int i = 0; i < n_fd; ++i)
            {
                if(events[i].events & EPOLLIN)//read,client close or down
                {
//                    if(fds_connections_.find(events[i].data.fd)!=fds_connections_.end())
//                    {
                        auto conn = fds_connections_.at(events[i].data.fd);
                        //printf("connection info%d",conn->connected_fd_);
                        server_->on_read_(conn);
//                    }
                }
                if(events[i].events & EPOLLOUT)  //connected also trigger
                {
                    if(fds_connections_.find(events[i].data.fd)!=fds_connections_.end())
                    {
                        auto conn=fds_connections_.at(events[i].data.fd);
                        if(conn->read)
                        {
                            conn->on_write_(conn);
                        }
                    }
                    //exit(7);
                }
                if(events[i].events & EPOLLRDHUP)
                {
                    if(fds_connections_.find(events[i].data.fd)!=fds_connections_.end())
                    {
                        auto conn = fds_connections_.at(events[i].data.fd);
                        //conn->Close();
                        //exit(8);
                    }
                }
                if(events[i].events & EPOLLERR)
                {
                    //exit(9);
                }
            }

        }
    };
    connection_thread_=thread(thread_fun);
}

void ConnectionThread::AddConnection(ConnectionPtr conn)
{
    {
        lock_guard<mutex> guard(mutex_new_);
        connections_new_.push_back(conn);
        fds_connections_[conn->connected_fd_]=conn;
    }
}

void ConnectionThread::DeleteConnection(ConnectionPtr conn)
{
    //printf(" delete\n");
    {
        lock_guard<mutex> guard(mutex_removed_);
        connections_removed_.push_back(conn);
//        if(fds_connections_.find(conn->connected_fd_)!=fds_connections_.end())
//        {
            fds_connections_.erase(fds_connections_.find(conn->connected_fd_));
//        }
    }
    //printf(" delete\n");
}

