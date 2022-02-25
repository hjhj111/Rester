#include "connection.h"
#include"connectionthread.h"

Connection::Connection(ResterServer* server)
    :server_(server)
{

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

    thread_->DeleteConnection(ConnectionPtr(this) );

}

void Connection::Init(ConnectionThread* thread)
{
    thread->AddConnection(GetShare());
    thread_=thread;
}

bool Connection::operator==(const Connection &other)
{
    return this->connected_fd_ == other.connected_fd_;
}
