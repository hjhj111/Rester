cmake_minimum_required(VERSION 3.21)
project(Rester)

set(DCMAKE_BUILD_TYPE,“Release”)

# 查找指定目录下的所有.cpp与.h文件 并存放到指定变量名SC_FILES中
FILE(GLOB SC_FILES "*.cpp" "*.h" "log/*.cpp" "log/*.h")
#FILE(GLOB SC_FILES "log/*.cpp" "log/*.h")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
add_executable(Rester ${SC_FILES} utls.cpp)
