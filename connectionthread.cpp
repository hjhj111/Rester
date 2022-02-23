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
#include<thread>
#include <algorithm>

ConnectionThread::ConnectionThread(int max_connection,GetCallBack on_get):
    max_connection_(max_connection),
    on_get_(on_get)
{

}

void ConnectionThread::Init()
{
    epoll_fd_ = epoll_create(max_connection_);

    auto thread_fun=[&]()
    {
        int ret=-1;
        while (true)
        {
            //printf("connection thread running\n");
            //qDebug()<<"connection thread running";
            //cout<<connections_new_.size()<<endl;
            if(!connections_removed_.empty())
            {
                lock_guard<mutex> guard(mutex_connections);
                for(auto conn:connections_removed_)
                {
                    ret=epoll_ctl(epoll_fd_, EPOLL_CTL_DEL,conn->connectioned_fd_ , &conn->event_);
                    if(ret<0)
                    {
                        perror("epoll remove wrong\n");
                        exit(-1);
                    }
                    connections_.remove(conn);
                    cout<<"remove connection in loop\n";
                }
                connections_removed_.clear();
                //cout<<"hhhhhhhhhhhhhhh"<<endl;
            }
            if(!connections_new_.empty())
            {
                //cout<<connections_new_.front()->connectioned_fd_<<endl;
                lock_guard<mutex> guard(mutex_connections);
                for(auto conn:connections_new_)
                {
                    /*设置成非阻塞模式*/
                    cout<<conn->connectioned_fd_<<endl;
                    int flags = fcntl(conn->connectioned_fd_, F_GETFL, 0);
                    if(flags < 0)
                    {
                          perror("fcntl F_GETFL");
                          exit(-1);
                    }
                    flags |= O_NONBLOCK;
                    ret = fcntl(conn->connectioned_fd_, F_SETFL, flags);
                    if(ret < 0)
                    {
                          perror("fcntl F_SETFL");
                          exit(-1);
                    }

                    ret=epoll_ctl(epoll_fd_, EPOLL_CTL_ADD,conn->connectioned_fd_ , &conn->event_);
                    if(ret<0)
                    {
                        perror("epoll add wrong\n");
                        exit(-1);
                    }
                    connections_.push_back(conn);
                    cout<<"add connection into loop\n";
                }
                connections_new_.clear();
            }


            epoll_event events[max_connection_];
            memset(events, 0x00, sizeof(epoll_event)*max_connection_);
            int nfds = epoll_wait(epoll_fd_, events, max_connection_, 1);
            //cout<<"epoll wait int connection thread "<<nfds<<"\\"<<sizeof(events)/sizeof(epoll_event)<<endl;
            //cout<<"connection nums "<<fds_connections_.size()<<endl;
            if(nfds == -1 )//&& errno == EINTR
            {
                exit(-55);
                continue;
            }
            for(int i = 0; i < nfds; ++i)
            {
                cout<<"event in connection thread "<<events[i].events<<endl;
                if(events[i].events & EPOLLIN)
                {
                    cout<<"recieved in connection thread\n";
                    on_get_(events[i].data.fd);
                    if(fds_connections_.find(events[i].data.fd)!=fds_connections_.end())
                    {
                        cout<<"delete"<<endl;
                        DeleteConnection(fds_connections_[events[i].data.fd]);
                        cout<<"delete over"<<endl;
                    }
                }

            }

        }
    };
    thread connection_thread(thread_fun);
    connection_thread.detach();
}

void ConnectionThread::AddConnection(ConnectionPtr conn)
{
    lock_guard<mutex> guard(mutex_connections);
    {
        connections_new_.push_back(conn);
        fds_connections_[conn->connectioned_fd_]=conn;
        cout<<"add connection\n";
        cout<<"connection nums "<<fds_connections_.size()<<endl;
    }
}

void ConnectionThread::DeleteConnection(ConnectionPtr conn)
{
    lock_guard<mutex> guard(mutex_connections);
    {
        if(std::find(connections_.begin(), connections_.end(),conn)!=connections_.end())
        {
            connections_removed_.push_back(conn);
        //fds_connections_.remove(conn.connectioned_fd_);
            fds_connections_.erase(fds_connections_.find(conn->connectioned_fd_));
            cout<<"delete connection"<<endl;
            cout<<"connection nums "<<fds_connections_.size()<<endl;
        }


    }
}
