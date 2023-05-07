//
// Created by hj on 2022/2/21.
//

#include "utls.h"

atomic<int> g_request_count{0};
atomic<int> g_connection_count{0};
atomic<int> g_add_and_delete_socket_time{0};
int g_recv_buf_size{1000};
int g_once_sent_size{8000};
