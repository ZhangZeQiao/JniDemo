# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

# TODO 设置构建本机库文件所需的 CMake的最小版本
cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# TODO 添加自己写的 C/C++源文件
add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp )

add_library( file-encrypt
             SHARED
             src/main/cpp/file-encrypt.cpp )

add_library( file-split-merge
             SHARED
             src/main/cpp/file-split-merge.cpp )

# TODO FFmpeg
add_library( ffmpeg_decode_demo
             SHARED
# TODO src/main/cpp/ffmpeg/ffmpeg.cpp ) -->  java.lang.UnsatisfiedLinkError: Native method not found
# TODO cpp文件不能跟文件夹重名，不然 System.loadLibrary()找不到
             src/main/cpp/ffmpeg/ffmpeg_decode_demo.cpp )
add_library( ffmpeg_player
             SHARED
             src/main/cpp/ffmpeg/ffmpeg_player.cpp )
# TODO 配置加载 native依赖
# TODO include_directories( src/main/jniLibs/${ANDROID_ABI}/include )
# TODO 路径指向上面会编译出错，指向下面的路径就没问题
include_directories( src/main/cpp/ffmpeg/include )
# TODO 添加第三方库
# TODO add_library(libavcodec-57
# TODO 原先生成的.so文件在编译后会自动添加上前缀lib和后缀.so，
# TODO       在定义库的名字时，不要加前缀lib和后缀 .so，
# TODO       不然会报错:java.lang.UnsatisfiedLinkError: Couldn't load xxx : findLibrary returned null
add_library(avcodec-57
            # TODO STATIC表示静态的.a的库，SHARED表示.so的库
            SHARED
            IMPORTED)
set_target_properties(avcodec-57
                      PROPERTIES IMPORTED_LOCATION
                      # TODO ${CMAKE_SOURCE_DIR}：表示 CMakeLists.txt的当前文件夹路径
                      # TODO ${ANDROID_ABI}：编译时会自动根据 CPU架构去选择相应的库
                      # TODO ABI文件夹上面不要再分层，直接就 jniLibs/${ANDROID_ABI}/
                      # TODO ${CMAKE_SOURCE_DIR}/src/main/jniLibs/ffmpeg/${ANDROID_ABI}/libavcodec-57.so
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec-57.so)
add_library(avdevice-57
            SHARED
            IMPORTED)
set_target_properties(avdevice-57
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavdevice-57.so)
add_library(avfilter-6
            SHARED
            IMPORTED)
set_target_properties(avfilter-6
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter-6.so)
add_library(avformat-57
            SHARED
            IMPORTED)
set_target_properties(avformat-57
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat-57.so)
add_library(avutil-55
            SHARED
            IMPORTED)
set_target_properties(avutil-55
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil-55.so)
add_library(postproc-54
            SHARED
            IMPORTED)
set_target_properties(postproc-54
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc-54.so)
add_library(swresample-2
            SHARED
            IMPORTED)
set_target_properties(swresample-2
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample-2.so)
add_library(swscale-4
            SHARED
            IMPORTED)
set_target_properties(swscale-4
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale-4.so)
# TODO libyuv是Google开源的实现各种YUV与RGB之间相互转换、旋转、缩放的库
add_library(yuv
            SHARED
            IMPORTED)
set_target_properties(yuv
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libyuv.so)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

# TODO 依赖 NDK中的库
find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

# TODO 将目标库与 NDK中的库进行连接
target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

target_link_libraries( file-encrypt
                       ${log-lib} )

target_link_libraries( file-split-merge
                       ${log-lib} )

# TODO 将目标库与 NDK中的库 及 用到的第三方库进行连接
target_link_libraries( ffmpeg_decode_demo

                       avcodec-57
                       avdevice-57
                       avfilter-6
                       avformat-57
                       avutil-55
                       postproc-54
                       swresample-2
                       swscale-4
                       ${log-lib} )

target_link_libraries( ffmpeg_player

                       # TODO https://stackoverflow.com/questions/43926204/android-undefined-reference-with-ndk
                       # TODO 解决  #include <android/native_window_jni.h> 时编译出现的错误
                       # TODO error:Could not find class 'android.graphics.xxx
                       -ljnigraphics
                       # TODO error:undefined reference to `ANativeWindow_fromSurface' 等等等
                       -landroid
                       # lib_opencv # Error:error: linker command failed with exit code 1 (use -v to see invocation)

                       avcodec-57
                       avdevice-57
                       avfilter-6
                       avformat-57
                       avutil-55
                       postproc-54
                       swresample-2
                       swscale-4
                       yuv
                       ${log-lib} )