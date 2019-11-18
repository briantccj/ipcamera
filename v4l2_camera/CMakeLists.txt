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
include_directories(/home/chechaojun/opencv/build/install/include/opencv4)


link_directories(
    
/mnt/hgfs/ubuntu_share/ipcamera/live555_linux/mediaServer/usb_camera/xvidcore/build/generic/build
../liveMedia/build
../groupsock
../BasicUsageEnvironment
../UsageEnvironment
../../x264
/usr/lib/x86_64-linux-gnu
/home/chechaojun/opencv/build/install/lib64
)

set(SOURCE_LIST 
    v4l2_camera_interface.cpp
#    v4l2grab.c
#    capture.c
#opencv.cpp
)

add_executable(${TARGET} ${SOURCE_LIST})

target_link_libraries(
    ${TARGET}
#    libopencv_world.so
    libm.so
    libpthread.so
    ${CMAKE_DL_LIBS}
)