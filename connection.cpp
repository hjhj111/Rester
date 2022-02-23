#include "connection.h"
#include"connectionthread.h"

Connection::Connection(ResterServer* server):
    server_(server)
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
    this->connectioned_fd_=connection.connectioned_fd_;
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
    this->connectioned_fd_=connection.connectioned_fd_;
    return *this;
}

void Connection::Init(ConnectionThread* thread)
{
    //push to thread;
    cout<<"connection init\n";
    thread->AddConnection(GetShare());
}

bool Connection::operator==(const Connection &other)
{
    return this->connectioned_fd_==other.connectioned_fd_;
}
