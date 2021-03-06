# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.


cmake_minimum_required(VERSION 3.10.1)

file(GLOB PATH src/main/cpp/*.cpp)
add_library( # Sets the name of the library.
        StephenFFmpeg
        SHARED
        ${PATH})


find_library( # Sets the name of the path variable.
        log-lib
        log)

set(distribution_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI})#将so库放入src/main/jniLibs目录下
include_directories(src/main/cpp)
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)


add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavcodec-58.so)

add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavfilter-7.so)

add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavformat-58.so)

add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libavutil-56.so)

add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libswresample-3.so)


add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/libswscale-5.so)

target_link_libraries( # Specifies the target library.
        StephenFFmpeg
        OpenSLES
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        z
        android
        ${log-lib}
        )




