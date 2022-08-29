#include<iostream>
#include<string>
#include"rester.h"

#define MYTEST

int main(int argc, char *argv[])
{
#ifdef MYTEST
    fclose(stdout);
#endif
    Config config;
    config.Update();
    cout<<config;

    Rester server(config);

    //close_log > 0 for close log
    if(!Log::get_instance()->init("log",0,8192,5000000,50000))
    {
        return 4;
    }

    Router file_worker("/");//default router
    auto on_get= [](RequestPtr request_ptr,ResponsePtr response_ptr)
    {
//            redis test
//            cpp_redis::client client;
//            client.connect("115.156.245.91",6379,
//                           [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status) {
//                if (status == cpp_redis::client::connect_state::ok) {
//                    std::cout << "client connected to " << host << ":" << port << std::endl;
//                }
//            });
//            cpp_redis::reply reply1;
//            client.get("hello", [&reply1](cpp_redis::reply& reply) {
//                reply1=reply;
//                std::cout << "get hello: " << reply << std::endl;
//                //exit(78);
//            });
//            client.sync_commit();
//            std::cout << "get hello: " << reply1 << std::endl;
//            printf("%s\n",reply1.as_string().c_str());
        response_ptr->SetStatusCode(200);
        response_ptr->SetHeader("Content-Type", "text/html"); //application/x-zip-compressed
        response_ptr->SetHeader("Connection", "keep-alive");
//            response.setHeader("Keep-Alive","timeout=1000");
//            response_ptr->setHeader("Content-Length", "15204315");

        response_ptr->PrintResponse();
        char *buf2 = nullptr;
        int length = ReadFileAll("2022_08_24_log", buf2);
        response_ptr->SetData(buf2, length);
        response_ptr->CombineResponse();
        printf("read file all %s body_length： %d， response_length: %d \n", response_ptr->HeadersStr().c_str(),length,response_ptr->ResponseLen());
    };
    file_worker.SetGet(on_get);

    //TODO on_post_ redis
    server.AddWorker(file_worker);

    printf("log inited\n");
    server.Init();
    return 0;
}
