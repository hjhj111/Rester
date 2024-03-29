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
            : url_(url)
    {
//        LOG_INFO("add router %s", url.c_str());
    }

    void SetGet(GetCallBack on_get)
    {
        on_get_ = on_get;
    }

    void SetPost(PostCallBack on_post)
    {
        on_post_ = on_post;
    }

    void SetDelete(DeleteCallBack on_delete)
    {
        on_delete_ = on_delete;
    }

    void SetOptions(OptionsCallBack on_options)
    {
        on_Options_=on_options;
    }

    //deprecated
    void SetConnect(ConnectCallBack on_connect)
    {
        on_connect_=on_connect;
    }

    void SetClose(CloseCallBack on_close)
    {
        on_close_=on_close;
    }

//    void OnGet(RequestPtr request_ptr, ResponsePtr response_ptr)
//    {
//        on_get_(request_ptr, response_ptr);
//    }
//
//    void OnPost(RequestPtr request_ptr, ResponsePtr response_ptr)
//    {
//        on_post_(request_ptr, response_ptr);
//    }
//
//    void OnDelete(RequestPtr request_ptr, ResponsePtr response_ptr)
//    {
//        on_delete_(request_ptr, response_ptr);
//    }
//
//    void OnOptions(RequestPtr request_ptr, ResponsePtr response_ptr)
//    {
//        on_Options_(request_ptr,response_ptr);
//    }

    GetCallBack OnGetFunc() const
    {
        return on_get_;
    }

    PostCallBack OnPostFunc() const
    {
        return on_post_;
    }

    DeleteCallBack OnDeleteFunc() const
    {
        return on_delete_;
    }

    OptionsCallBack  OnOptionsFunc() const
    {
        return on_Options_;
    }

    CloseCallBack OnCloseFunc() const
    {
        return on_close_;
    }

    ConnectCallBack OnConnectFunc() const
    {
        return on_connect_;
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
    DeleteCallBack on_delete_;
    OptionsCallBack  on_Options_;

    ConnectCallBack on_connect_;//connection callback can not be set
    // before data read from client and router is chosen,so it is no meaning to set on_connect_.
    // it is universal, and set by rester
    CloseCallBack  on_close_;

    //condition_variable conv;
};


#endif //RESTER_ROUTER_H
