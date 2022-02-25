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

class UrlWorker
{
public:
    UrlWorker(string url)
    :url_(url)
    {
        on_get_ = [](ConnectionPtr conn)
        {//TODO send_file_once to connection

            printf("on write\n");
            int fd = conn->connected_fd_;
            int &response_size = conn->buf_size_;
            char *&response_buf = conn->buf_;
            int &sent_size = conn->sent_size_;

            request_count++;

            char head[] = "HTTP/1.1 200 OK\r\n"
                          //"Transfer-Encoding: chunked\r\n"//Content-Type: text/html  Transfer-Encoding: chunked
                          //"Content-Type: text/html\r\n"
                          "Content-Type: application/x-zip-compressed\r\n"
                          "Connection: keep-alive\r\n"
                          "Keep-Alive: timeout=1000\r\n"
                          "Content-Length: 15204315\r\n"
                          "\r\n";
            int length;
            printf("sent_size%d\n", sent_size);
            if (sent_size == 0)
            {
                char *buf2 = "";
                length = read_file_all("file.zip", buf2);
                response_size = length + sizeof(head);
                response_buf = new char[response_size];
                snprintf(response_buf, response_size, "%s%s", head, buf2);
                printf("read file all %d\n", length);
            }
            if (sent_size > response_size)
            {
                //sent_size=0;
                printf("sent size %d\n", sent_size);
                exit(11);
            } else if (sent_size == response_size)
            {
                conn->Close();
            }
            //sent_size=0;
            while (sent_size < response_size)
            {
                int left = response_size - sent_size;
                if (left > chunk_size)
                {
                    left = chunk_size;
                }
                int ret = send(fd, response_buf + sent_size, left, MSG_DONTWAIT);
                if (ret <= 0)
                {
                    break;
                    perror("send wrong\n");
                    //LOG_ERROR("send wrong int on_get");
                } else
                {
                    sent_size += ret;
                }
                //printf("send %d\n",ret);

            }
        };

        //TODO on_post_ redis
    }

    string url_;
    //callback
    WriteCallBack  on_get_;
    PostCallBack on_post_;
};


#endif //RESTER_URLWORKER_H
