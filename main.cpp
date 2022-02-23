#include<iostream>
#include<string>
//#include"rapidjson/allocators.h"
#include"utls.h"
#include"resterserver.h"

using namespace std;

//extern Config config;
#define MYTEST

int main(int argc, char *argv[])
{
#ifdef MYTEST
    fclose(stdout);
    fclose(stderr);
#endif
    Config config;
    config.Update();
    cout<<config;

    ResterServer server(config);

    server.Init();

}
