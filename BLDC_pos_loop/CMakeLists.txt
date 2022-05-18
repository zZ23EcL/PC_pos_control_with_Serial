cmake_minimum_required(VERSION 3.22)
project(BLDC_pos_loop)

set(CMAKE_CXX_STANDARD 14)

add_executable(BLDC_pos_loop main.cpp SerialPort.h SerialPort.cpp MiniPID.h MiniPID.cpp)
