#include<iostream>
#include<string>
//#include"utls.h"
#include"resterserver.h"

using namespace std;

//#define MYTEST

int main(int argc, char *argv[])
{
#ifdef MYTEST
    fclose(stdout);
#endif
    Config config;
    config.Update();
    cout<<config;

    ResterServer server(config);

    if(!Log::get_instance()->init("log",1,8192,5000000,50000))
    {
        return 4;
    }

    UrlWorker file_worker("/");
    //worker.on_get_
    server.AddWorker(file_worker);

    printf("log inited\n");
    server.Init();
    return 0;
}
