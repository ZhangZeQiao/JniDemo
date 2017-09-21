package com.xq.jnidemo;

/**
 * @author 小侨
 * @time 2017/9/21  18:11
 * @desc NDK错误排查工具
 */

public class JNIErrorCheck {

    /*
    NDK排查错误
    1、保存logcat文件，分析日志
    2、ndk-stack工具
    adb logcat | ndk-stack -sym J:/mars-jee/dn_ffmpeg_player/obj/local/armeabi/
    3、addr2line定位错误行
    arm-linux-androideabi-addr2line -e J:/mars-jee/dn_ffmpeg_player/obj/local/armeabi/libmyffmpeg.so 0x580001d
    工具位置：D:\sdk\ndk-bundle\toolchains\arm-linux-androideabi-4.9\prebuilt\windows-x86_64\bin
    */
}
