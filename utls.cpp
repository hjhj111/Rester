//
// Created by hj on 2022/2/21.
//

#include "utls.h"

atomic<int> request_count{0};
atomic<int> connection_count{0};
int buf_size{1000};

