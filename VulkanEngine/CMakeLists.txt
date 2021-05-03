cmake_minimum_required(VERSION 3.7.1)
project(vienna_vulkan_engine_cmake)

set(CMAKE_CXX_STANDARD 20)


find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)


include_directories(
        ../external/stb
        ../external/threadpool

        #brew headers
        /usr/local/include
)
link_directories(
        #brew lib
        /usr/local/lib
)

# add the executable
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "*.h" "*.cpp")
add_executable(
        app
        ${SOURCE_FILES}
        ../external/threadpool/ThreadPool.h
        ../external/threadpool/ThreadPool.cpp
)

target_include_directories(app PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})
target_link_libraries(app PRIVATE vulkan glfw assimp ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} ${SWSCALE_LIBRARY})
