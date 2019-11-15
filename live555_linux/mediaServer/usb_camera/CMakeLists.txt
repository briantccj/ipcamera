
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(usb_camera)
set(TARGET usb_camera)

add_definitions(-std=c++11)
add_definitions(-lm)


include_directories(/mnt/hgfs/ubuntu_share/ipcamera/live555_linux/mediaServer/usb_camera/xvidcore/src)
include_directories(../liveMedia/include)
include_directories(../groupsock/include)
include_directories(../UsageEnvironment/include)
include_directories(../live555_rtsp_live_v4l2-master/include)
include_directories(../../x264)


link_directories(
    
/mnt/hgfs/ubuntu_share/ipcamera/live555_linux/mediaServer/usb_camera/xvidcore/build/generic/build
../liveMedia/build
../groupsock
../BasicUsageEnvironment
../UsageEnvironment
../../x264
/usr/lib/x86_64-linux-gnu
)

set(SOURCE_LIST 
    ubuntu_encode_usb_camera.c
)

add_executable(${TARGET} ${SOURCE_LIST})

target_link_libraries(
    ${TARGET}
    libxvidcore.a
    libm.so
    libpthread.so
    ${CMAKE_DL_LIBS}
)