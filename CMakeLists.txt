cmake_minimum_required (VERSION 2.8)
project (ida_dcu)

set(OUTSTATION_VERSION 1.0.0)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX_FLAGS})
include_directories(modbus /usr/local/include/modbus/)
include_directories(../deps/inih)
#include_directories(mariadb /usr/include/mariadb)
#include_directories(/home/hamed/WiringNP/wiringPi/)

# ----- ida_dcu executable -----
file(GLOB_RECURSE main_SRC src/*.cpp src/*.h)
add_executable (ida_dcu deps/inih/ini.c deps/inih/ini.h ${main_SRC})
target_link_libraries (ida_dcu LINK_PUBLIC pthread modbus mariadb -lwiringPi ) 
