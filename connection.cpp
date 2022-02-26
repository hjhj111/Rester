#include "connection.h"
#include"connectionthread.h"

Connection::Connection(ResterServer* server)
    :server_(server)
{
    on_write_=[](ConnectionPtr conn)
    {
        printf("null on_write triggered\n");
    };
}

Connection::Connection(const Connection &connection)
{
    this->event_=connection.event_;
    this->ip_=connection.ip_;
    this->port_=connection.port_;
    this->is_on_=connection.is_on_;
    this->state_=connection.state_;
    this->thread_=connection.thread_;
    this->server_=connection.server_;
    this->connected_fd_=connection.connected_fd_;
    //return this;
}

Connection &Connection::operator=(const Connection &connection)
{
    this->event_=connection.event_;
    this->ip_=connection.ip_;
    this->port_=connection.port_;
    this->is_on_=connection.is_on_;
    this->state_=connection.state_;
    this->thread_=connection.thread_;
    this->server_=connection.server_;
    this->connected_fd_=connection.connected_fd_;
    return *this;
}

void Connection::Close()
{
    printf(" close\n");
    thread_->DeleteConnection(GetShare() );
}

void Connection::Init(ConnectionThread* thread)
{
    thread_=thread;
    thread->AddConnection(GetShare());

}

bool Connection::operator==(const Connection &other)
{
    return this->connected_fd_ == other.connected_fd_;
}
