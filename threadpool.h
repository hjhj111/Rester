#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<vector>
#include"utls.h"
//#include"resterconnection.h"
//#include"connectionthread.h"

//#include"resterserver.h"

using namespace std;

class ConnectionThread;

class ThreadPool
{
public:
    ThreadPool();
    void Init(int max_thread,int max_connection,GetCallBack on_get);
    ConnectionThread* GetThread();
private:
    int max_thread_;
    int max_connection_;
    vector<ConnectionThread*> threads_;
    int thread_ind_=0;
};

#endif // THREADPOOL_H
