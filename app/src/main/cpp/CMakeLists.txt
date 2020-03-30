# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
include_directories(/src/main/cpp/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/armeabi-v7a}")
cmake_minimum_required(VERSION 3.10.1)

add_library( # Sets the name of the library.
        native-lib
        SHARED
        native-lib.cpp)

find_library( # Sets the name of the path variable.
        log-lib
        log)

target_link_libraries( # Specifies the target library.
        native-lib
        avcodec avfilter avformat avutil swresample swscale
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})