package com.xq.jnidemo.ffmpeg;

import android.view.Surface;

/**
 * @author 小侨
 * @time 2017/9/13  16:11
 * @desc 采用 ffmpeg的视频操作类
 */

public class VideoUtils {

    static {
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("yuv");
        System.loadLibrary("ffmpeg_decode_demo");
        System.loadLibrary("ffmpeg_player");
    }

    public native static void decode(String input, String output);

    public native static void player(String input_jstr, String output_jstr);

    public native static void render(String input_jstr, Surface surface);

}
