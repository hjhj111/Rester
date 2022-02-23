#include "threadpool.h"
#include "connectionthread.h"

ThreadPool::ThreadPool()
{
//    cout<<"thread pool \n";
}

void ThreadPool::Init(int max_thread,int max_connection,GetCallBack on_get)
{
    max_thread_=max_thread;
    max_connection_=max_connection;
    for(int i=0;i<max_thread;i++)
    {
//        cout<<"thread init"<<endl;
        auto th=new ConnectionThread(max_connection_,on_get);
        th->Init();
        threads_.push_back(th);
    }
    cout<<"threadpool init\n";
//    printf("threadpool init\n");
}

ConnectionThread *ThreadPool::GetThread()
{
//    threads.front()->Init();
    cout<<"get thread\n";
    return threads_[thread_ind_++%max_thread_];
}
