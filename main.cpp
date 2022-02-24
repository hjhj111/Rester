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
    //fclose(stderr);
#endif
    Config config;
    config.Update();
    cout<<config;

    ResterServer server(config);
    if(!Log::get_instance()->init("",0))
    {
        return 11;
    }

    server.Init();
    return 0;
}
