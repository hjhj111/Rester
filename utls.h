#ifndef UTLS_H
#define UTLS_H
#include<string>
//#include<iostream>
#include<fstream>
#include<filesystem>
#include<functional>
#include<atomic>
#include <string>
#include <sstream>
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
using GetCallBack=function<void(ConnectionPtr)>;
using WriteCallBack=function<void(ConnectionPtr)>;
using PostCallBack=function<void(ConnectionPtr)>;
using CloseCallBack=function<void(ConnectionPtr)>;

extern atomic<int> request_count;
extern atomic<int> connection_count;
extern atomic<int> add_and_delete_socket_time;
extern int buf_size;
extern int chunk_size;

inline std::string dec2hex(int i, int width)
{
    std::stringstream ioss;     //定义字符串流
    std::string s_temp;         //存放转化后字符
    ioss << std::hex << i;      //以十六制形式输出
    ioss >> s_temp;

    if(width > s_temp.size())
    {
        std::string s_0(width - s_temp.size(), '0');      //位数不够则补0
        s_temp = s_0 + s_temp;                            //合并
    }

    std::string s = s_temp.substr(s_temp.length() - width, s_temp.length());    //取右width位
    return s;
}

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

inline int read_file_all(const char* file_path_name, char* & buf)
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

inline int get_file_size(ifstream& f_read)
{
    f_read.seekg(0,ios::end);
    int length=f_read.tellg();
    f_read.seekg(0,ios::beg);
    return length;
}

inline int read_file_part(ifstream& f_read, int part_size ,char* & buf)
{
    f_read.read(buf,part_size);
}

inline pair<int,int> get_file_chunk(const char* file_path_name, char*& buf,char* head,int head_size)
{
    ifstream f_read(file_path_name);
    if(f_read.fail())
    {
        printf("file not open: ");
        return make_pair(-1,-1);
    }
    int length= get_file_size(f_read)-1;

    int n=length/chunk_size+1;
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
            int left=length-(n-1)*chunk_size;
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
            f_read.read(buf+ind,chunk_size);
            ind+=chunk_size;
            char en[]="\r\n";
            strcpy(buf+ind,en);
            ind+=2;
        }
        //printf("%d\n",i);
    }
    //printf("hhhhhhhhhhhhhh");
    f_read.close();

    //buf_length= strlen(buf);
    buf_length=ind;
    return make_pair(length,buf_length);

}

inline int get_id()
{
    long long time_ms=chrono::duration_cast<chrono::milliseconds>
            (chrono::high_resolution_clock::now().time_since_epoch()).count();
    hash<long long> ha;
    return ha(time_ms);
}

inline void print_raw(char* raw, int size)
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
 * */

#endif // UTLS_H
