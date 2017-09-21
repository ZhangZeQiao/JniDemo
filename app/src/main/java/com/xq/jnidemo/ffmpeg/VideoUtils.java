package com.xq.jnidemo.ffmpeg;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
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
        System.loadLibrary("ffmpeg_video_player");
        System.loadLibrary("ffmpeg_audio_player");
    }

    public native static void decode(String input, String output);

    public native static void playerDecode(String input_jstr, String output_jstr);

    public native static void render(String input_jstr, Surface surface);

    public native static void soundDecode(String input_jstr, String output_jstr);

    // 喇叭只支持PCM格式，播放的音频文件最终都要解码成PCM格式才能播放
    // 手机自带播发器不一定能解码所有的音频文件，一般只支持常用的音频文件（MP3等）
    // 但是FFmpeg几乎支持所有音频文件（视频文件）的解码，这就是用FFmpeg的原因

    // Android中两种播放音频的方式：AudioTrack、OpenSL

    /**
     * 创建一个AudioTrac对象，用于播放声音
     *
     * @param nb_channels 声道个数
     * @return
     */
    public static AudioTrack createAudioTrack(int sample_rate, int nb_channels) {
        // 固定格式的音频码流
        int sampleRateInHz = sample_rate;
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        //声道布局
        int channelConfig;
        if (nb_channels == 1) {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if (nb_channels == 2) {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO; // 立体声
        } else {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO; // 立体声
        }

        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
        AudioTrack audioTrack = new AudioTrack(
                AudioManager.STREAM_MUSIC,
                sampleRateInHz,
                channelConfig,
                audioFormat,
                bufferSizeInBytes,
                AudioTrack.MODE_STREAM);

        // 播放
        // audioTrack.play();
        // 写入PCM
        // audioTrack.write(byte[]buffer);

        return audioTrack;
    }

    public native static void soundPlay(String input_jstr, String output_jstr);

    // 音视频同步播放
    public native static void play(String input_jstr, Surface surface);
}
