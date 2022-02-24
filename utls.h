#ifndef UTLS_H
#define UTLS_H
#include<string>
//#include<iostream>
#include<fstream>
#include<filesystem>
#include<functional>
#include<atomic>
#include <chrono>

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

#include"rapidjson/allocators.h"
#include"rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include "TimeCount.h"
#include "log/log.h"

using namespace std;
using namespace rapidjson;

struct Config
{
    string root_folder;
    int port;
    int max_connection;
    int max_thread;
    void Update()
    {
        ifstream fp;
        fp.open("/home/hj/CLionProjects/Rester/config.json",ios_base::in);
        if(fp.fail())
        {
            printf("config file doesn't exist\n");
            exit(-1);
        }
        string raw_str;
        char ch;
        while(fp.get(ch))
        {
            raw_str.push_back(ch);
        }
        Document d;
        d.Parse(raw_str.c_str());

    //    FILE* fp = fopen("config.json", "r"); // 非 Windows 平台使用 "r"
    //    char readBuffer[65536];
    //    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    //    Document d;
    //    d.ParseStream(fp);

        if(d.HasParseError())
        {
            printf("parse json error\n");
        }

        if(!d.HasMember("root_folder"))
        {
            printf("no member root_folder\n");
            exit(-1);
        }
        root_folder=d["root_folder"].GetString();

        if(!d.HasMember("max_thread"))
        {
            printf("no member max_thread\n");
            exit(-1);
        }
        max_thread=d["max_thread"].GetInt();

        if(!d.HasMember("max_connection"))
        {
            printf("no member max_connection\n");
            exit(-1);
        }
        max_connection=d["max_connection"].GetInt();

        if(!d.HasMember("port"))
        {
            printf("no member port\n");
            exit(-1);
        }
        port=d["port"].GetInt();


        printf("config: %s\n", raw_str.c_str());
        cout<<"result of json parsing---------"<<endl;
        fp.close();
        //fclose(fp);
    }

};

inline ostream& operator <<(ostream& os, const Config& config)
{
    os<<"config: \n";
    os<<"port: "<<config.port<<endl;
    os<<"root_folder: "<<config.root_folder<<endl;
    os<<"max_connection: "<<config.max_connection<<endl;
    os<<"max_thread: "<<config.max_thread<<endl;
    return os;
}

//extern Config config;
class Connection;
using ConnectionPtr=shared_ptr<Connection>;
using ConnectCallBack=function<void(ConnectionPtr)>;
using GetCallBack=function<void(int)>;
using PostCallBack=function<void(int)>;
using CloseCallBack=function<void(int)>;

extern atomic<int> request_count;
extern atomic<int> connection_count;
extern atomic<int> add_and_delete_socket_time;
extern int buf_size;

inline bool recv_once(int fd,char buf[],int& size_read)
{
    size_read=0;
    int ind=0;
    int n_try=0;
    bool re=true;
    //link=true;
    while (true)
    {
        n_try++;
        int ret = recv(fd,buf+ind,buf_size-ind,MSG_DONTWAIT);

        if (ret==-1)
        {
            //printf("%s\n", strerror(errno));
            if (size_read>0)// //(errno == EAGAIN||errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                re=false;
                break;
            }
        }
        else if(ret==0)
        {
            re=false;
            break;
        }
        else
        {
            size_read+=ret;
        }
    }
    return re;
}

inline int get_id()
{
    long long time_ms=chrono::duration_cast<chrono::milliseconds>
            (chrono::high_resolution_clock::now().time_since_epoch()).count();
    hash<long long> ha;
    return ha(time_ms);
}

/*exit code 1 listen fd error
 * 2 client fd error
 * 3 listen fd epoll error
 * */

#endif // UTLS_H
