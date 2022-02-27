//
// Created by hj on 2022/2/25.
//

#ifndef RESTER_URLWORKER_H
#define RESTER_URLWORKER_H

#include <string>
using namespace  std;

#include "cpp_redis/cpp_redis"

#include "utls.h"
#include "connectionthread.h"
#include "connection.h"
#include "http-parser/httpresponse.h"

//#pragma comment(lib,"libcpp_redis.a" "libtacopie.a")

class UrlWorker
{
public:
    explicit UrlWorker(string url)
    :url_(move(url))
    {
        on_get_ = [](RequestPtr request_ptr,ResponsePtr response_ptr)
        {
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
            //std::cout << "get hello: " << reply1 << std::endl;
            //printf("%s\n",reply1.as_string().c_str());
            response_ptr->setStatusCode(200);
            response_ptr->setHeader("Content-Type", "application/x-zip-compressed");
            response_ptr->setHeader("Connection", "keep-alive");
            //response.setHeader("Keep-Alive","timeout=1000");
            response_ptr->setHeader("Content-Length", "15204315");

            response_ptr->printResponse();
            char *buf2 = nullptr;
            int length = read_file_all("index.html", buf2);
            response_ptr->setData(buf2, length);
            response_ptr->combineResponse();
            printf("read file all %s %d %d \n", response_ptr->getHead().c_str(),length,response_ptr->m_response_len);
        };

        //TODO on_post_ redis
    }

    bool PathMath(string path)
    {
        //if(path.find())
    }

    string url_;
    //callback
    GetCallBack  on_get_;
    PostCallBack on_post_;
};


#endif //RESTER_URLWORKER_H
