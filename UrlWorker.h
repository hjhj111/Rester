//
// Created by hj on 2022/2/25.
//

#ifndef RESTER_URLWORKER_H
#define RESTER_URLWORKER_H

#include <string>
using namespace  std;

#include "utls.h"
#include "connectionthread.h"
#include "connection.h"
#include "http-parser/httpresponse.h"

class UrlWorker
{
public:
    explicit UrlWorker(string url)
    :url_(move(url))
    {
        on_get_ = [](RequestPtr request_ptr,ResponsePtr response_ptr)
        {
            //int &response_size = conn->buf_size_;
            //char *& response_buf = conn->buf_;
            //ResponsePtr response_ptr= make_shared<Response>();
            response_ptr->setStatusCode(200);
            response_ptr->setHeader("Content-Type", "application/x-zip-compressed");
            response_ptr->setHeader("Connection", "keep-alive");
            //response.setHeader("Keep-Alive","timeout=1000");
            response_ptr->setHeader("Content-Length", "15204315");

            response_ptr->printResponse();
            char *buf2 = nullptr;
            int length = read_file_all("file.zip", buf2);
            response_ptr->setData(buf2, length);
            response_ptr->combineResponse();
            //auto head= response.getHead();//attention straight return c_str() will get nothing
            // because const right string will be deleted right soon
            //auto head_len= response.getHeadLen();

            //response_size=response_ptr->getResponseLen();
            //response_buf=response_ptr->getResponse();

            //response_buf=head.c_str();
//            int ret=snprintf(response_buf, response_size, "%s%s", head.c_str(), buf2);
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
