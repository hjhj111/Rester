#include <arpa/inet.h>

#include "rester.h"
#include "utls.h"
#include "connection.h"
#include "connections_thread.h"
#include "http-parser/HttpParser.h"

Rester::Rester(const Config& config)
    :max_connection_(config.max_connection),
      max_thread_(config.max_thread),
      server_port_(config.port)
{
    on_connect_=[](ConnectionPtr conn)
    {
        printf("on connected\n");
        struct in_addr tmp;
        tmp.s_addr=conn->ip_;
        auto str_ip=inet_ntoa(tmp);
        LOG_INFO("new connection:%s： %d",str_ip,conn->port_);
    };

    on_read_=[](ConnectionPtr conn)
    {
        printf("on read\n");
        int fd=conn->connected_fd_;
        int size_read=0;
        bool link=true;
        char buf[1000];

        link= RecvOnce(fd, buf, size_read);
        if(!link||size_read==0)//&&size_read==0 mean connection closed by client
        {
            printf("recv once %d %d \n",link,size_read);
            conn->Close();
        }
        else
        {
            printf("%s",buf);
            LOG_INFO(buf);
            conn->request_ptr_=make_shared<HttpParser>(buf,size_read);
            auto request_ptr=conn->request_ptr_;
//            parser.show();
//            printf("key1: %s  key2: %s\n",parser.GetUrlParameter("key1").c_str(),
//                   parser.GetUrlParameter("key2").c_str());
            auto url=(*request_ptr)["url"];
            printf("url: %s\n",url.c_str());
            auto& url_workers=conn->server_->url_workers_;
            conn->server_->PrintWorkers();
            if(url_workers.find(url) != url_workers.end())
            {
                conn->on_close_=url_workers.at(url).OnCloseFunc();
                if((*request_ptr)["method"] == "GET")
                {
                    auto on_get=url_workers.at(url).OnGetFunc();
                    conn->SetOnWrite(on_get);
                }
                else if((*request_ptr)["method"] == "POST")
                {
                    auto on_post=url_workers.at(url).OnPostFunc();
                    conn->SetOnWrite(on_post);
                }
                else if((*request_ptr)["method"] == "DELETE")
                {
                    auto on_delete=url_workers.at(url).OnDeleteFunc();
                    conn->SetOnWrite(on_delete);
                }
                else if((*request_ptr)["method"] == "OPTIONS")
                {
                    auto on_options=url_workers.at(url).OnOptionsFunc();
                    conn->SetOnWrite(on_options);
                }
                else
                {
                    printf("url %s does not have this http method %s",url.c_str(),
                           (*request_ptr)["method"].c_str());
                    conn->Close();
                    //exit(21);
                }
            }
            else
            {
                printf("no url %s", url.c_str());
                conn->Close();
                //exit(22);
            }
            //response data ready
            conn->read=true;
            //add EPOLLOUT event for client fd
            auto ev=conn->event_.events;
            ev|=EPOLLOUT;
            conn->event_.events=ev;
            auto ret=epoll_ctl(conn->thread_->epoll_fd_, EPOLL_CTL_MOD, conn->connected_fd_ , &conn->event_);
        }
    };

    on_write_=[](ConnectionPtr conn)
    {
        printf("on write\n");
        int fd = conn->connected_fd_;

        int response_size = conn->response_ptr_->ResponseLen();
        char * response_buf = conn->response_ptr_->ResponseData();
        int &sent_size = conn->sent_size_;
        g_request_count++;
        printf("sent size %d %d %d\n", sent_size ,response_size,conn->response_ptr_->ResponseLen());

//            char head[] = "HTTP/1.1 200 OK\r\n"
//                          //"Transfer-Encoding: chunked\r\n"//Content-Type: text/html  Transfer-Encoding: chunked
//                          //"Content-Type: text/html\r\n"
//                          "Content-Type: application/x-zip-compressed\r\n"
//                          "Connection: keep-alive\r\n"
//                          "Keep-Alive: timeout=1000\r\n"
//                          "Content-Length: 15204315\r\n"
//                          "\r\n";



//      printf("sent_size%d\n", sent_size);
        if (sent_size == 0)
        {
//          set response_size and response_buf
            conn->on_write_( conn->request_ptr_,conn->response_ptr_);
            //update response
            response_size = conn->response_ptr_->ResponseLen();
            response_buf = conn->response_ptr_->ResponseData();
        }
        if (sent_size > response_size)
        {
            //printf("sent size %d \\ %d %d\n", sent_size ,response_size,conn->response_ptr_->ResponseLen());
            exit(11);
        }
        else if (sent_size == response_size)
        {
            conn->Close();
            return;
        }
        //sent_size=0;
        while (sent_size < response_size)
        {
            int left = response_size - sent_size;
            if (left > g_once_sent_size)
            {
                left = g_once_sent_size;

            }
            int ret = send(fd, response_buf + sent_size, left, MSG_DONTWAIT);
            printf("in while sent size %d, response_size %d\n",sent_size, response_size);
            if (ret <= 0)
            {
                //printf("in while sent size %d, response_size %d\n",sent_size, response_size);
                perror("send wrong\n");
                //LOG_ERROR("send wrong int on_get");
                break;
            }
            else
            {
                sent_size += ret;
                printf("send %d %d\n",ret,sent_size);
                if(sent_size==response_size)
                {
                    printf("send over\n");
//                    int z;
//                    default option: send as far as possible, but no recv
//                    linger so_linger;
//                    so_linger.l_onoff=0;
//                    z= setsockopt(fd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof so_linger);
//                    if(z)
//                    {
//                        perror("setsockoption solinger:");
//                    }
                    conn->Close();
                    return;
                }
            }


        }
        //printf("in while sent size %d, response_size %d\n",sent_size, response_size);
    };

    thread_pool_=new ThreadPool(this);
    thread_pool_->Init(max_thread_,max_connection_);
}

void Rester::Init()
{
    struct sockaddr_in s_addr, c_addr;
    int ret=-1;
    //server address
    s_addr.sin_family = AF_INET;
    s_addr.sin_port   = htons(server_port_);
    if(!inet_aton("0.0.0.0", (struct in_addr *) &s_addr.sin_addr.s_addr))
    {
        perror("invalid ip address:");
        exit(1);
    }
    //server socket
    if((listen_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket create failed:");
        exit(1);
    }
    //端口重用，调用close(socket)一般不会立即关闭socket，而经历“TIME_WAIT”的过程
    int reuse = 0x01;
    if(setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0)
    {
        perror("set sock option error");
        close(listen_fd_);
        exit(1);
    }
    //设置成非阻塞模式
    int flags = fcntl(listen_fd_, F_GETFL, 0);
    if(flags < 0)
    {
          perror("fcntl F_GETFL");
          exit(1);
    }
    flags |= O_NONBLOCK;
    ret = fcntl(listen_fd_, F_SETFL, flags);
    if(ret < 0)
    {
          perror("fcntl");
          exit(1);
    }

    //绑定地址
    if(bind(listen_fd_, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0)
    {
        perror("bind error");
        close(listen_fd_);
        exit(1);
    }
    //监听socket
    if(listen(listen_fd_, max_connection_*max_thread_) < 0)
    {
        perror("listen error");
        close(listen_fd_);
        exit(1);
    }

    const int listen_n=1;//////////////////////////////////////////////////////////
    socklen_t cin_len = sizeof(struct sockaddr_in);
    epoll_event events[listen_n];
    epoll_event event;
    memset(&c_addr, 0x00, sizeof(sockaddr_in));
    memset(events, 0x00, sizeof(epoll_event)*listen_n );
    memset(&event, 0x00, sizeof(epoll_event));
    epoll_fd_ = epoll_create(listen_n);
    if(epoll_fd_==-1)
    {
        perror("epoll_fd_");
        exit(1);
    }

    event.events = EPOLLIN|EPOLLET;
    event.data.fd = listen_fd_;
    ret=epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event);  //监听接收
    if(ret==-1)
    {
        perror("epoll add");
        exit(1);
    }
    //main eventloop
    while(running_)
    {
        int nfds = epoll_wait(epoll_fd_, events, listen_n, 5);
        if(nfds == -1 && errno == EINTR)
        {
            running_=false;
        }
        for(int i = 0; i < nfds; ++i)
        {
            //printf("main loop");
            if(events[i].data.fd == listen_fd_)  //监听新连接
            {
                while(true)
                {
                    int nfd = accept(listen_fd_,(struct sockaddr*)&c_addr,&cin_len);
                    if(nfd == -1)
                    {
                        printf("failed accept, error = %s\n", strerror(errno));
                        //continue;
                        break;
                    }
                    g_connection_count++;

//                    LOG_INFO("new connection %s:%d",inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port))
                    auto connection=make_shared<Connection>(this);
                    connection->connected_fd_ = nfd;
                    //connection->ip_ = ntohl(c_addr.sin_addr.s_addr);
                    connection->ip_ = c_addr.sin_addr.s_addr;//keep net byte order
                    connection->port_ = ntohs(c_addr.sin_port);
                    connection->is_on_ = false;
                    epoll_event ev;
                    ev.events = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLERR;//no EPOLLOUT first, and then add after read
                    ev.data.fd = nfd;
                    connection->event_=ev;

                    on_connect_(connection);//for rester
                    connection->OnConnect();//for specific connection, but no data, no router, no specific on_connect
                    ConnectionsThread* thread=thread_pool_->GetThread();
                    connection->Init(thread);//connection detach  new connection go to connection thread;
                }
            }
        }
    }

}

void Rester::AddWorker(const Router &worker)
{
    url_workers_.insert(make_pair(worker.Url(), worker));
}