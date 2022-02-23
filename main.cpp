#include<iostream>
#include<string>
//#include"rapidjson/allocators.h"
#include"config.h"
#include"resterserver.h"

using namespace std;

//extern Config config;

int main(int argc, char *argv[])
{
    fclose(stdout);
    fclose(stderr);
    Config config;
    config.Update();
    cout<<config;

    ResterServer server(config);

    server.Init();

}
