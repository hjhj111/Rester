#include "threadpool.h"
#include "connectionthread.h"

ThreadPool::ThreadPool()
{
//    cout<<"thread pool \n";
//    cout.flush();

}

void ThreadPool::Init(int max_thread,int max_connection,GetCallBack on_get)
{

    max_thread_=max_thread;
    max_connection_=max_connection;
    for(int i=0;i<max_thread;i++)
    {
//        cout<<"thread init"<<endl;
//        cout.flush();
        auto th=new ConnectionThread(max_connection_,on_get);
        th->Init();
        threads.push_back(th);
    }
    cout<<"threadpool init\n";
    //printf("threadpool init\n");

}

ConnectionThread *ThreadPool::GetThread()
{
    //threads.front()->Init();
    cout<<"get thread\n";
    return threads.front();
}
