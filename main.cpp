#include<iostream>
#include<string>
#include <signal.h>
#include"rester.h"

//#define MYTEST
shared_ptr<Rester> server_ptr;

void SigFunc(int sig)
{
    printf("sigfunc %d\n",sig);
    server_ptr->running_=false;
    signal(SIGINT,SIG_DFL);
}

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);//peer close while server sending , ignore and close
#ifdef MYTEST
    fclose(stdout);
    fclose(stderr);
#endif
    Config config;
    config.Update();
    cout<<config;

    server_ptr=make_shared<Rester>(config);

    //close_log > 0 for close log
    if(!Log::get_instance()->init("log",0,8192,5000000,10000))
    {
        return 4;
    }
    printf("log inited\n");

    auto& connection_pool=ConnectionPool::GetInstance();//auto can not carry & itself

    Router note_book("/notes");//default router

    auto on_get= [&connection_pool](RequestPtr request_ptr,ResponsePtr response_ptr)
    {
        //auto client =ConnectToRedis();
        auto client=connection_pool.GetClient();
        ClientHelper client_helper(connection_pool,client);//for automatic return client

        auto notes_size=client->llen("notes_list");
        client->sync_commit();
        auto si=notes_size.get().as_integer()-1;
        std::cout<<si<<std::endl;
        cpp_redis::reply reply1;
        client->lrange("notes_list",0,si,[&reply1](cpp_redis::reply& reply){
            if(reply.ok())
            {
                reply1=reply;
                std::cout << "get from redis: " << reply.is_array() << std::endl;
            }
            else
            {
                cout<<"error: "<<reply1.error()<<endl;
            }
        });
        client->sync_commit();

        string ret;
        auto arr=reply1.as_array();
        ret+="[";
        for(int i=0;i<arr.size();i++)
        {
            auto& re=arr[i];
            auto re_str=re.as_string();
            ret+=re_str;
            if(i==arr.size()-1)
            {
                continue;
            }
            ret+=",";
        }
        ret+="]";
        printf("list from redis: %s\n",ret.c_str());
        response_ptr->SetStatusCode(200);
        response_ptr->SetHeader("Content-Type", "application/json"); //application/x-zip-compressed
        response_ptr->SetHeader("Connection", "keep-alive");
        response_ptr->SetHeader("Access-Control-Allow-Origin","*");
        response_ptr->SetHeader("Allow","GET,POST,DELETE");
        response_ptr->PrintResponse();

        int length=-1;
        length=ret.length();
        auto buf2=ret.c_str();
        response_ptr->SetData(buf2, length);
        response_ptr->CombineResponse();
        printf("read file all %s body_length： %d， response_length: %d \n", response_ptr->HeadersStr().c_str(),length,response_ptr->ResponseLen());
    };

    //no redis
    auto on_get_no_redis= [](RequestPtr request_ptr,ResponsePtr response_ptr)
    {
        string ret="hello rester";
        response_ptr->SetStatusCode(200);
        response_ptr->SetHeader("Content-Type", "application/json"); //application/x-zip-compressed
        response_ptr->SetHeader("Connection", "keep-alive");
        response_ptr->SetHeader("Access-Control-Allow-Origin","*");
        response_ptr->SetHeader("Allow","GET,POST,DELETE");
        response_ptr->PrintResponse();

        int length=-1;
        length=ret.length();
        auto buf2=ret.c_str();
        response_ptr->SetData(buf2, length);
        response_ptr->CombineResponse();
        printf("gen response done\n headers: %s\nbody_length： %d， response_length: %d \n", response_ptr->HeadersStr().c_str(),length,response_ptr->ResponseLen());

    };

    note_book.SetGet(on_get_no_redis);

    auto on_post=[&connection_pool](RequestPtr request_ptr,ResponsePtr response_ptr)
    {
        auto client=connection_pool.GetClient();
        ClientHelper client_helper(connection_pool,client);//for automatic return client
        cpp_redis::reply reply1;
        cout<<"body: "<<(*request_ptr)["body"]<<(*request_ptr)["body"].size()<<endl;
        client->lpush("notes_list",vector<string>{(*request_ptr)["body"]},[response_ptr](cpp_redis::reply& reply){
            if(reply.ok())
            {
                response_ptr->SetStatusCode(201);
                response_ptr->SetHeader("Content-Type", "application/json"); //application/x-zip-compressed
                response_ptr->SetHeader("Access-Control-Allow-Origin","*");
                response_ptr->PrintResponse();
                response_ptr->CombineResponse();
            }
            else
            {
                response_ptr->SetStatusCode(406);
                response_ptr->SetHeader("Content-Type", "application/json"); //application/x-zip-compressed
                response_ptr->SetHeader("Access-Control-Allow-Origin","*");
                response_ptr->PrintResponse();
                response_ptr->CombineResponse();
            }
        });
        client->sync_commit();
        //printf("%s\n",reply1.as_string().c_str());
        //client->shutdown();
    };
    note_book.SetPost(on_post);

    auto on_options=[](RequestPtr request_ptr,ResponsePtr response_ptr)
    {
        response_ptr->SetStatusCode(200);
        response_ptr->SetHeader("Content-Type", "application/json"); //application/x-zip-compressed
        response_ptr->SetHeader("Access-Control-Allow-Origin","*");
        response_ptr->SetHeader("Access-Control-Allow-Headers","*");
        response_ptr->SetHeader("Access-Control-Allow-Methods","GET,POST,OPTIONS,DELETE");
        response_ptr->PrintResponse();
        response_ptr->CombineResponse();
    };
    note_book.SetOptions(on_options);

    auto on_delete=[&connection_pool](RequestPtr request_ptr,ResponsePtr response_ptr)
    {
//        auto client =ConnectToRedis();
        auto client=connection_pool.GetClient();
        ClientHelper client_helper(connection_pool,client);//for automatic return client
        cpp_redis::reply reply1;
        cout<<"body: "<<(*request_ptr)["body"]<<(*request_ptr)["body"].size()<<endl;
        client->lrem("notes_list",0,(*request_ptr)["body"],[response_ptr](cpp_redis::reply& reply){
            if(reply.ok())
            {
                cout<<"delete ok"<<endl;
                response_ptr->SetStatusCode(201);
                response_ptr->SetHeader("Content-Type", "application/json"); //application/x-zip-compressed
                response_ptr->SetHeader("Access-Control-Allow-Origin","*");
                response_ptr->PrintResponse();
                response_ptr->CombineResponse();
            }
            else
            {
                cout<<"delete error"<<endl;
                response_ptr->SetStatusCode(406);
                response_ptr->SetHeader("Content-Type", "application/json"); //application/x-zip-compressed
                response_ptr->SetHeader("Access-Control-Allow-Origin","*");
                response_ptr->PrintResponse();
                response_ptr->CombineResponse();
            }
        });
        client->sync_commit();
    };
    note_book.SetDelete(on_delete);

    auto on_close=[](ConnectionPtr conn)
    {
        struct in_addr tmp;
        tmp.s_addr=conn->ip_;
        auto str_ip=inet_ntoa(tmp);
//        LOG_INFO("connection closed:%s： %d",str_ip,conn->port_);
        //cout<<"close loginfo"<<endl;
    };
    note_book.SetClose(on_close);

    server_ptr->AddWorker(note_book);

    signal(SIGINT,SigFunc);
    server_ptr->Init();
    return 0;
}
