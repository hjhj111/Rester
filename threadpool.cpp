#include "threadpool.h"
//#include "connectionthread.h"

void ThreadPool::Init(int max_thread, int max_connection)
{
    max_thread_=max_thread;
    max_connection_=max_connection;
    for(int i=0;i<max_thread;i++)
    {
        auto th=new ConnectionThread(max_connection_);
        th->Init();
        threads_.push_back(th);
    }
//    printf("threadpool init\n");
}

ConnectionThread *ThreadPool::GetThread()
{
    return threads_[thread_ind_++%max_thread_];
}
