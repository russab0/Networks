enable_testing()

find_package(Threads REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})

add_executable(node node.c)

target_link_libraries(node ${CHECK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
