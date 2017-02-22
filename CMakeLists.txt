cmake_minimum_required(VERSION 2.8)
project(Hearthplay)

find_package( Threads )

file(GLOB SOURCES "Hearthplay/*.cpp" "Hearthplay/*.h")
 
add_executable(hearthplay ${SOURCES})
target_link_libraries( hearthplay ${CMAKE_THREAD_LIBS_INIT} )
