//
// Created by hj on 2022/2/25.
//

#ifndef RESTER_ROUTER_H
#define RESTER_ROUTER_H

#include <string>

#include "cpp_redis/cpp_redis"

#include "utls.h"
#include "connections_thread.h"
#include "connection.h"
#include "http-parser/httpresponse.h"

//#pragma comment(lib,"libcpp_redis.a" "libtacopie.a")

class Router
{
public:
    explicit Router(string url)
    :url_(move(url))
    {
        LOG_INFO("add router %s",url.c_str());
    }

    void SetGet(GetCallBack on_get)
    {
        on_get_=on_get;
    }

    void OnGet(RequestPtr request_ptr,ResponsePtr response_ptr)
    {
        on_get_(request_ptr,response_ptr);
    }

    GetCallBack OnGetFunc() const
    {
        return on_get_;
    }

    PostCallBack OnPostFunc() const
    {
        return on_post_;
    }

    string Url() const
    {
        return url_;
    }

private:
    string url_;
    //callback
    GetCallBack  on_get_;
    PostCallBack on_post_;
};


#endif //RESTER_ROUTER_H
