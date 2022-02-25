#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<vector>

//#include"utls.h"
#include"connectionthread.h"

using namespace std;

class ThreadPool
{
public:
    ThreadPool()
    {

    }

    ~ThreadPool()
    {
        for(auto connection_thread:threads_)
        {
            delete connection_thread;
        }
    }

    void Init(int max_thread,int max_connection);

    ConnectionThread* GetThread();

private:
    int max_thread_;
    int max_connection_;
    vector<ConnectionThread*> threads_;
    int thread_ind_=0;
};

#endif // THREADPOOL_H
