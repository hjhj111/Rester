#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<vector>

//#include"utls.h"
#include"connections_thread.h"

//using namespace std;

class ThreadPool
{
public:
    ThreadPool(Rester* server)
    :server_(server)
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

    ConnectionsThread* GetThread();

private:
    Rester* server_;
    int max_thread_;
    int max_connection_;
    vector<ConnectionsThread*> threads_;
    int thread_ind_=0;
};

#endif // THREADPOOL_H
