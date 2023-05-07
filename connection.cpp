#include "connection.h"
#include"connections_thread.h"

Connection::Connection(Rester* server)
    :server_(server)
{
    response_ptr_= make_shared<Response>();
    on_write_=[](RequestPtr request_ptr,ResponsePtr response_ptr)
    {
        printf("null on_write triggered\n");
    };
}

//Connection::Connection(const Connection &connection)
//{
//    this->event_=connection.event_;
//    this->ip_=connection.ip_;
//    this->port_=connection.port_;
//    this->is_on_=connection.is_on_;
//    this->state_=connection.state_;
//    this->thread_=connection.thread_;
//    this->server_=connection.server_;
//    this->connected_fd_=connection.connected_fd_;
//    //return this;
//}
//
//Connection &Connection::operator=(const Connection &connection)
//{
//    this->event_=connection.event_;
//    this->ip_=connection.ip_;
//    this->port_=connection.port_;
//    this->is_on_=connection.is_on_;
//    this->state_=connection.state_;
//    this->thread_=connection.thread_;
//    this->server_=connection.server_;
//    this->connected_fd_=connection.connected_fd_;
//    return *this;
//}

void Connection::Close()
{
    printf("close\n");
    OnClose();
    thread_->DeleteConnection(GetShare() );
}

void Connection::AddToThread(ConnectionsThread* thread)
{
    thread_=thread;
    thread->AddConnection(GetShare());
}

bool Connection::operator==(const Connection &other)
{
    return this->connected_fd_ == other.connected_fd_;
}

void Connection::SetOnWrite(GetCallBack on_write)
{
    on_write_=on_write;
}

void Connection::SendOver()
{
    printf("send over\n");
    sent_size_=0;
    read=false;
    //request_ptr_= nullptr;
    //response_ptr_= nullptr;
    response_ptr_= make_shared<Response>();
    RemoveEpollOut();
    if(ShortConnection)
    {
        Close();
    }
}

void Connection::AddEpollOut()
{
    auto ev=event_.events;
    ev=EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLERR|EPOLLOUT;
    event_.events=ev;
    auto ret=epoll_ctl(thread_->epoll_fd_, EPOLL_CTL_MOD, connected_fd_ , &event_);
    if(ret<0)
    {
        perror("epoll mod err: ");
    }
    else
    {
        printf("add epollout\n");
    }
}
void Connection::RemoveEpollOut()
{
    auto ev=event_.events;
    ev=EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLERR;
    event_.events=ev;
    auto ret=epoll_ctl(thread_->epoll_fd_, EPOLL_CTL_MOD, connected_fd_ , &event_);
    if(ret<0)
    {
        perror("epoll mod err: ");
    }
    else
    {
        printf("remove epollout\n");
    }
//    if(event_.events & EPOLLOUT)
//    {
//        printf("EPOLLOUT not removed\n");
//    }
//    else
//    {
//        printf("EPOLLOUT is removed\n");
//    }
}
