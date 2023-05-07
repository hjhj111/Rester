#ifndef UTLS_H
#define UTLS_H
#include<string>
#include<fstream>
#include<filesystem>
#include<functional>
#include<atomic>
#include <sstream>
#include <chrono>

#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<cstdio>// stdio.h was deprecated
#include<cstdlib> // stdlib.h was deprecated
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include <sys/epoll.h>

#include"rapidjson/allocators.h"
#include"rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include "cpp_redis/cpp_redis"

#include "time_count.h"
#include "log/log.h"
#include "http-parser/HttpParser.h"
#include "http-parser/httpresponse.h"
#include "ConnectionPool.h"

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

class Connection;
using ConnectionPtr=shared_ptr<Connection>;

using ConnectCallBack=function<void(ConnectionPtr)>;
using WriteCallBack=function<void(ConnectionPtr)>;
using CloseCallBack=function<void(ConnectionPtr)>;
using ReadCallBack=function<void(ConnectionPtr)>;

using GetCallBack=function<void(RequestPtr,ResponsePtr)>;
using PostCallBack=function<void(RequestPtr,ResponsePtr)>;
using DeleteCallBack=function<void(RequestPtr,ResponsePtr)>;
using OptionsCallBack=function<void(RequestPtr,ResponsePtr)>;



extern atomic<int> g_request_count;
extern int g_recv_buf_size;
extern int g_once_sent_size;

inline bool RecvOnce(int fd, char buf[], int& size_read)
{
    size_read=0;
    int ind=0;
    int n_try=0;
    bool ret=true;
    while (true)
    {
        n_try++;
        int ret = recv(fd,buf+ind, g_recv_buf_size - ind, MSG_DONTWAIT);
        if (ret<0)
        {
            printf("%s\n", strerror(errno));
            if (errno == EAGAIN||errno == EWOULDBLOCK)
            {
                //continue;
                perror("eagain in readn(mean client closed)");
                break;
            }
            else
            {
                perror("other error in readn");
                ret=false;
                break;
            }
        }
        else if(ret==0)
        {
            perror("client closed\n");
            ret=false;
            break;
        }
        else
        {
            size_read+=ret;
        }
    }
    return ret;
}

inline int ReadFileAll(const char* file_path_name, char* & buf)
{
    ifstream f_read(file_path_name);
    if(f_read.fail())
    {
        printf("file not open: ");
        return -1;
    }
    f_read.seekg(0,ios::end);
    int length=f_read.tellg();
    f_read.seekg(0,ios::beg);
    buf=new char[length];
    f_read.read(buf,length);
    f_read.close();
    return length;
}

inline int GetFileSize(ifstream& f_read)
{
    f_read.seekg(0,ios::end);
    int length=f_read.tellg();
    f_read.seekg(0,ios::beg);
    return length;
}

inline void ReadFilePart(ifstream& f_read, int part_size , char* & buf)
{
    f_read.read(buf,part_size);
}


//for connection id
inline int GetUid()
{
    long long time_ms=chrono::duration_cast<chrono::milliseconds>
            (chrono::high_resolution_clock::now().time_since_epoch()).count();
    hash<long long> ha;
    return ha(time_ms);
}

//for test bytes of http message
inline void PrintRaw(char* raw, int size)
{
    for(int i=0;i<size;i++)
    {
        char c=raw[i];
        if(c=='\r')
        {
            printf("%c",'r');
        }
        else if(c=='\n')
        {
            printf("%c\n",'n');
        }
        else if(c=='\0')
        {
            printf("end");
        }
        else if(isdigit(c))
        {
            printf("%c",c);
        }
    }
}

/*exit code 1 listen fd error
 * 2 client fd error
 * 3 listen fd epoll error
 * 4 log init error
 * 5 log full and push error
 * 2x url or method or parameter or bodydata error
 * */

#endif // UTLS_H
