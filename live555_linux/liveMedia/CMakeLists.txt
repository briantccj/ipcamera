
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(liveMedia)
set(TARGET liveMedia)

add_definitions(-std=c++11)
add_definitions(-fpermissive)

include_directories(../BasicUsageEnvironment/include)
include_directories(../liveMedia/include)
include_directories(../groupsock/include)
include_directories(../UsageEnvironment/include)


link_directories(
../liveMedia
../groupsock
../BasicUsageEnvironment
../UsageEnvironment
)

file(GLOB SOURCE_PATH  *.cpp  *.c)
add_library(${TARGET}
            STATIC
            ${SOURCE_PATH} )