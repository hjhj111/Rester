#include "resterserver.h"
#include "config.h"
#include "resterconnection.h"
#include "connectionthread.h"
#include "httpresponse.h"

//extern Config config;

ResterServer::ResterServer(const Config& config)
    :max_connection_(config.max_connection),
      max_thread_(config.max_thread)
{
//    max_connection_=config.max_connection;
//    max_thread_=config.max_thread;
    //thread_pool_=ThreadPool(max_thread_);
    on_connected=[](Connection* conn)
    {
        cout<<conn->ip_<<endl;
        cout.flush();
    };
    on_get=[](int fd)
    {
        request_count++;
        char buf[1000];
        char tosend[]="HTTP/1.1 200 OK\r\n"
                      "\r\n"
                      "gggggggg";
        cout<<"on_get  fd: "<<fd<<endl;
        int size_read=0;
        bool link=true;
        link= recv_once(fd,buf,size_read);
        //int ret= recv(fd,buf,1000,0);
        cout<<request_count<<endl;
        if(!link||size_read==0)
        {
            cout<<"not link "<<size_read<<endl;
            //close(fd);
            return;
            //exit(89);
        }
        cout<<"linkkkkkkkkkkkkkkkkkkkkkkkkk"<<size_read<<endl;
        int ret=send(fd,tosend,sizeof(tosend),MSG_DONTWAIT);
        //int ret= write(fd,tosend,sizeof(tosend))
        if(ret<=0)
        {
            perror("send wrong\n");
            //exit(-1);
        }
        cout<<ret<<"send over\n";
        //close(fd);
    };
    thread_pool_=ThreadPool();
    thread_pool_.Init(max_thread_,max_connection_,on_get);

}

bool ResterServer::Init()
{
    struct sockaddr_in s_addr, c_addr;
    int ret=-1;
    /* 服务器端地址 */
    s_addr.sin_family = AF_INET;
    s_addr.sin_port   = htons(5000);
    if(!inet_aton("0.0.0.0", (struct in_addr *) &s_addr.sin_addr.s_addr))
    {
        perror("invalid ip addr:");
        exit(1);
    }
    /* 创建socket */
    if((listen_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket create failed:");
        exit(1);
    }
    /* 端口重用，调用close(socket)一般不会立即关闭socket，而经历“TIME_WAIT”的过程 */
    int reuse = 0x01;
    if(setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0)
    {
        perror("setsockopt error");
        close(listen_fd_);
        exit(1);
    }
    /*设置成非阻塞模式*/
    int flags = fcntl(listen_fd_, F_GETFL, 0);
    if(flags < 0)
    {
          perror("fcntl F_GETFL");
          exit(-1);
    }
    flags |= O_NONBLOCK;
    ret = fcntl(listen_fd_, F_SETFL, flags);
    if(ret < 0)
    {
          perror("fcntl");
          exit(-1);
    }

    /* 绑定地址 */
    if(bind(listen_fd_, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0)
    {
        perror("bind error");
        close(listen_fd_);
        exit(-1);
    }
    /* 监听socket */
    if(listen(listen_fd_, max_connection_*max_thread_) < 0)
    {
        perror("listen error");
        close(listen_fd_);
        exit(-1);
    }

    const int listen_n=1;//////////////////////////////////////////////////////////
    socklen_t cin_len = sizeof(struct sockaddr_in);
    epoll_event events[listen_n];
    epoll_event event;
    memset(&cin, 0x00, sizeof(sockaddr_in));
    memset(events, 0x00, sizeof(epoll_event)*listen_n );
    memset(&event, 0x00, sizeof(epoll_event));
    epoll_fd_ = epoll_create(listen_n);
    if(epoll_fd_==-1)
    {
        perror("epoll_fd_");
        exit(-1);
    }

    event.events = EPOLLIN|EPOLLET;
    event.data.fd = listen_fd_;
    ret=epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event);  //监听接收
    if(ret==-1)
    {
        perror("epoll add");
        exit(-1);
    }

    while(true)
    {
        int nfds = epoll_wait(epoll_fd_, events, listen_n, 1);
        //cout<<"server epoll go "<<nfds<<endl;

        if(nfds == -1 && errno == EINTR)
        {
            exit(-99);
            continue;
        }
        for(int i = 0; i < nfds; ++i)
        {
            if(events[i].data.fd == listen_fd_)  //监听新连接
            {
                while(true)
                {
                    int nfd = accept(listen_fd_,(struct sockaddr*)&cin,&cin_len);
                    if(nfd == -1)
                    {
                        printf("failed accept, error = %s\n", strerror(errno));
                        //continue;
                        break;
                    }
                    cout<<"new connection______________________________________________________"<<nfd<<endl;
                    connection_count++;
                    cout<<"connection count "<<connection_count<<endl;
                    auto connection=make_shared<Connection>(this);
                    connection->connectioned_fd_ = nfd;
                    connection->ip_ = ntohl(c_addr.sin_addr.s_addr);
                    connection->port_ = ntohs(c_addr.sin_port);
                    connection->is_on_ = false;
                    //connection.on_get_=[](int fd){cout<<fd<<endl;};
                    epoll_event ev;
                    ev.events = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLOUT|EPOLLERR;
                    ev.data.fd = nfd;
                    connection->event_=ev;
                    on_connected(connection.get());

                    ConnectionThread* thread=thread_pool_.GetThread();
//                thread->on_get_=[](int fd){
//                char buf1[]="hhhhhh";
//                //write(fd,buf1,10);
//                //cout<<fd<<"00000000000000000000000000000"<<endl;
//                };
                    connection->Init(thread);//connection detach  new connection go to connection thread;

                    //getchar();
                    //connections_.push_back(connection->GetShare());

                    //event.events = EPOLLIN|EPOLLHUP|EPOLLET;
                    //event.data.fd = nfd;
                    //epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, nfd, &event);//加入监听连接队列
                }



//                char buf[1000];
//                int s=0;
//                bool link=recv_once(nfd,buf,s);
//                close(nfd);
//                continue;

//                char buf[1000];
//                int s=0;
//                bool link=recv_once(nfd,buf,s);
//                buf[s]='\0';
//                cout<<buf<<endl;
//                link=false;
//                if(link)
//                {
//                    cout<<"+++++++++++++++++++++++++++++++++++\n";
//
//                    char tosend[]="HTTP/1.1 200 OK\r\n"
//                                  "\r\n"
//                                  "gggggggg";
//
//                    //usleep(100000);
//                    ret=send(nfd,tosend,sizeof(tosend),0);
//                    cout<<"send ret             "<<ret<<endl;
//                    //usleep(10000);
//                    if(ret<=0)
//                    {
//                        perror("send wrong\n");
//                        //exit(-1);
//                    }
//                }
//                else
//                {
//                    cout<<"-----------------------------------------\n";
//                }
//
//                close(nfd);
//                continue;

//                //test
//                //ret=recv(nfd,buf,1000,MSG_DONTWAIT);
//                bool link=true;
//                ret= recv_once(nfd,buf,link);
//                if(!link)
//                {
//                    cout<<"not link "<<ret<<endl;
//                    close(nfd);
//                    continue;
//                    //exit(89);
//                }
//                cout<<ret<<endl;
//                ret=send(nfd,tosend,sizeof(tosend),MSG_DONTWAIT);
//                if(ret<=0)
//                {
//                    perror("send wrong\n");
//                    //exit(-1);
//                }
//                cout<<ret<<"send over\n";
//                close(nfd);
//                continue;




            }
            //cout<<"hhhhhhhhhhhhhhh"<<endl;
        }
    }
}
