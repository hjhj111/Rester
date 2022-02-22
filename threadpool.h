#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<list>
#include"config.h"
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
    list<ConnectionThread*> threads;
};

#endif // THREADPOOL_H
