cmake_minimum_required (VERSION 3.10)
cmake_policy(SET CMP0016 NEW)

project(AsynTCP)

include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Boost 1.8 REQUIRED COMPONENTS system)

add_executable(server ${CMAKE_SOURCE_DIR}/src/server/serverok.cpp)
target_link_libraries(server Boost::system)

add_executable(client ${CMAKE_SOURCE_DIR}/src/client/client.cpp)
target_link_libraries(client Boost::system)