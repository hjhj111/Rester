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
extern atomic<int> g_connection_count;
extern atomic<int> g_add_and_delete_socket_time;
extern int g_recv_buf_size;
extern int g_once_sent_size;

inline bool RecvOnce(int fd, char buf[], int& size_read)
{
    size_read=0;
    int ind=0;
    int n_try=0;
    bool ret=true;
    //link=true;
    while (true)
    {
        n_try++;
        int ret = recv(fd,buf+ind, g_recv_buf_size - ind, MSG_DONTWAIT);

        if (ret==-1)
        {
            printf("%s\n", strerror(errno));
            if (size_read>0)// //(errno == EAGAIN||errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                ret=false;
                break;
            }
        }
        else if(ret==0)
        {
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

//for multipart/form
//TODO
inline pair<int,int> GetFileChunk(const char* file_path_name, char*& buf, char* head, int head_size)
{
    ifstream f_read(file_path_name);
    if(f_read.fail())
    {
        printf("file not open: ");
        return make_pair(-1,-1);
    }
    int length= GetFileSize(f_read) - 1;

    int n= length / g_once_sent_size + 1;
    printf("file length %d chunk %d\n",length,n);
    int buf_length=length+5000;
    buf=new char[buf_length];
    strcpy(buf,head);
    int ind=head_size;
    char gap[]={'2','7','1','0','\r','\n'};//"1\r\n"; //10000
    for(int i=0;i<n;i++)
    {
        //printf("%d\n",i);
        if(i==n-1)
        {
            int left=length- (n-1) * g_once_sent_size;
//            int left_bit=0;
//            int left1=left;
//            while(true)
//            {
//                if((left1/=10)>0)
//                {
//                    left_bit+=1;
//                }
//                else
//                {
//                    left_bit+=1;
//                    break;
//                }
//            }
            if(left>0)
            {
                char gap_last[7];
                snprintf(gap_last,sizeof(gap_last),"%s\r\n", "17fc");//dec2hex(left,4).c_str()
                strcpy(buf+ind,gap_last);
                ind+= strlen(gap_last);
                f_read.read(buf+ind,left);
                ind+=left;
            }


            char en[]="0\r\n\r\n";
            strcpy(buf+ind,en);
            ind+=5;
        }
        else
        {
            strcpy(buf+ind,gap);
            ind+=sizeof(gap);
            f_read.read(buf+ind, g_once_sent_size);
            ind+=g_once_sent_size;
            char en[]="\r\n";
            strcpy(buf+ind,en);
            ind+=2;
        }
        //printf("%d\n",i);
    }
    f_read.close();

    //buf_length= strlen(buf);
    buf_length=ind;
    return make_pair(length,buf_length);

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

//TODO connections  pool
inline cpp_redis::client* ConnectToRedis()
{
    static cpp_redis::client client;
    if(!client.is_connected())
    {
        client.connect("192.168.56.1", 6379,//115.156.245.91
                       [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status)
                       {
                           if (status == cpp_redis::client::connect_state::ok)
                           {
                               std::cout << "client connected to " << host << ":" << port << std::endl;
                           }
                       });
    }
   return &client;
}


// wrong when declared here for http header checking in httpresponse.h
//bool StrSame(const std::string& s1, const std::string& s2)
//{
//    if(s1.size()!=s2.size())
//    {
//        return false;
//    }
//    for(int i=0;i<s1.size();i++)
//    {
//        if(s1[i]!=s2[i]&&std::abs(s1[i]-s2[i])!=26)
//        {
//            return false;
//        }
//    }
//    return true;
//}

/*exit code 1 listen fd error
 * 2 client fd error
 * 3 listen fd epoll error
 * 4 log init error
 * 5 log full and push error
 * 2x url or method or parameter or bodydata error
 * */

#endif // UTLS_H
