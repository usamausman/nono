cmake_minimum_required(VERSION 3.1)
project(nono)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC src/*.cpp)
file(GLOB HDR src/*.hpp)

set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(nono ${SRC} ${HDR})
target_link_libraries(nono ${CURSES_LIBRARIES})