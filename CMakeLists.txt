cmake_minimum_required(VERSION 3.8.2)

project(sort-server VERSION 1.0.0)

find_package(Threads)

add_executable(client client.cpp 
                      proto/udp.cpp
                      proto/tcp.cpp
                      proto/event_listener.cpp
                      handler.cpp)

add_executable(server server.cpp 
                      handler.cpp
                      proto/event_listener.cpp 
                      proto/udp.cpp
                      proto/tcp.cpp)

set_target_properties(client server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
)

target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})

################################################################################

enable_testing()

add_executable(test_handler    handler_test.cpp handler.cpp)
add_executable(test_serializer proto/serializer_test.cpp)

set_target_properties(test_handler test_serializer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
)

add_test(handler    test_handler)
add_test(serializer test_serializer)
