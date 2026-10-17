cmake_minimum_required(VERSION 3.25)
project(cpfp_install_consumer LANGUAGES CXX)

find_package(cpfp 0.1 CONFIG REQUIRED)

add_executable(cpfp_install_consumer main.cpp)
target_link_libraries(cpfp_install_consumer PRIVATE cpfp::cpfp)
