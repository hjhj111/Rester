//
// Created by hj on 2022/9/2.
//

#include "ConnectionPool.h"

ConnectionPool &ConnectionPool::GetInstance()
{
    static ConnectionPool connection_pool;
    return connection_pool;
}

int ConnectionPool::size_=1000;