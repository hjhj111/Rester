//
// Created by hj on 2022/9/2.
//

#ifndef RESTER_CONNECTIONPOOL_H
#define RESTER_CONNECTIONPOOL_H

#include <list>
#include "cpp_redis/cpp_redis"

class ConnectionPool
{
public:
    //ConnectionPool()=delete;
    ConnectionPool& operator=(const ConnectionPool&)=delete;
    ConnectionPool(const ConnectionPool&)=delete;

    ~ConnectionPool()
    {
        for(auto& cli:clients_)
        {
            cli->shutdown();
            delete cli;
        }
    }

    static ConnectionPool& GetInstance();

    static void SetSize(int size)
    {
        size_=size;
    }

    static int Size()
    {
        return size_;
    }

    cpp_redis::client* GetClient()
    {
        std::lock_guard<std::mutex> guard_clients(mut_clients_);
        if(!clients_.empty())
        {
            auto cli= clients_.front();
            clients_.pop_front();
            return cli;
        }
        else
        {
            return nullptr;
        }
    }

    void ReturnClient(cpp_redis::client* cli)
    {
        std::lock_guard<std::mutex> guard_clients(mut_clients_);
        clients_.push_back(cli);
    }

private:
    ConnectionPool()
    {
        for(int i=0;i<size_;i++)
        {
            cpp_redis::client* client=new cpp_redis::client;
            client->connect("115.156.245.91", 6379,//115.156.245.91    192.168.56.1
               [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status)
               {
                   if (status == cpp_redis::client::connect_state::failed)
                   {
                       std::cout<<"connect failed: "<<(int)status<<std::endl;
                       exit(-20);
                   }
                   else if(status == cpp_redis::client::connect_state::ok)
                   {
//                       std::cout << "client connected to " << host << ":" << port
//                       <<"  connection status: "<<(int)status<< std::endl;
                   }
               });
            clients_.push_back(client);
        }
    }
    std::mutex mut_clients_;
    std::list<cpp_redis::client*> clients_;
    static int size_;

};

class ClientHelper
{
public:
    ClientHelper(ConnectionPool& connection_pool,cpp_redis::client* cli)
    :cli_(cli),
    connection_pool_(connection_pool)
    {

    }
    ~ClientHelper()
    {
        connection_pool_.ReturnClient(cli_);
    }
private:
    cpp_redis::client* cli_;
    ConnectionPool& connection_pool_;
};

#endif //RESTER_CONNECTIONPOOL_H
