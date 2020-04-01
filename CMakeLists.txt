cmake_minimum_required(VERSION 3.7)
project(osf_usb)

set(CMAKE_CXX_STANDARD 17)

add_executable(osf_usb test/main.cpp)
target_include_directories(osf_usb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
