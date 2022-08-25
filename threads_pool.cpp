#include "threads_pool.h"
//#include "connectionthread.h"

void ThreadPool::Init(int max_thread, int max_connection)
{
    max_thread_=max_thread;
    max_connection_=max_connection;
    for(int i=0;i<max_thread;i++)
    {
        auto th=new ConnectionsThread(server_, max_connection_);
        th->Init();
        threads_.push_back(th);
    }
//    printf("threadpool init\n");
}

ConnectionsThread *ThreadPool::GetThread()
{
    return threads_[thread_ind_++%max_thread_];
}
