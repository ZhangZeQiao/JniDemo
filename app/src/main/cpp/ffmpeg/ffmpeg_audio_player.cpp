//
// Created by joe on 2017/9/18.
//

#include <jni.h>
#include <stdlib.h>
#include <unistd.h>
#include "video_utils.h"

#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"xq",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"xq",FORMAT,##__VA_ARGS__);

#define MAX_AUDIO_FRME_SIZE 48000 * 4 // 缓冲区

extern "C" {
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_soundDecode
        (JNIEnv *env, jclass type, jstring input_jstr, jstring output_jstr) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, 0);
    const char *output_cstr = env->GetStringUTFChars(output_jstr, 0);

    LOGI("%s", "sound");
    //注册组件
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //打开音频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGI("%s", "无法打开音频文件");
        return;
    }

    //获取输入文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGI("%s", "无法获取输入文件信息");
        return;
    }

    //获取音频流索引位置
    int i = 0, audio_stream_idx = -1;
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }

    //获取解码器
    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if (codec == NULL) {
        LOGI("%s", "无法获取解码器");
        return;
    }
    //打开解码器
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        LOGI("%s", "无法打开解码器");
        return;
    }

    //压缩数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = 44100;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = codecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx,
                       out_ch_layout, out_sample_fmt, out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sample_rate,
                       0, NULL);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end

    //16bit 44100 PCM 数据 【缓冲区再大一点也是可以的，小了就不行】
    //uint8_t *out_buffer = (uint8_t *) av_malloc(2*44100);
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

    FILE *fp_pcm = fopen(output_cstr, "wb");

    int got_frame = 0, index = 0, ret;
    //不断读取压缩数据
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码
        ret = avcodec_decode_audio4(codecCtx, frame, &got_frame, packet);

        if (ret < 0) {
            LOGI("%s", "解码完成");
        }
        //解码一帧成功
        if (got_frame > 0) {
            LOGI("解码：%d", index++);
            swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,
                        (const uint8_t **) frame->data, frame->nb_samples);
            //获取sample的size
            int out_buffer_size = av_samples_get_buffer_size(
                    NULL, out_channel_nb,
                    frame->nb_samples, out_sample_fmt, 1);
            fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
        }

        av_free_packet(packet);
    }

    fclose(fp_pcm);
    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrCtx);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
    env->ReleaseStringUTFChars(output_jstr, output_cstr);
}

// TODO --------------------------------------------------------------------------------------------

// 基于上面的代码进行修改

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_soundPlay
        (JNIEnv *env, jclass type, jstring input_jstr, jstring output_jstr) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, 0);
    const char *output_cstr = env->GetStringUTFChars(output_jstr, 0);

    LOGI("%s", "sound");
    //注册组件
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //打开音频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGI("%s", "无法打开音频文件");
        return;
    }

    //获取输入文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGI("%s", "无法获取输入文件信息");
        return;
    }

    //获取音频流索引位置
    int i = 0, audio_stream_idx = -1;
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }

    //获取解码器
    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if (codec == NULL) {
        LOGI("%s", "无法获取解码器");
        return;
    }
    //打开解码器
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        LOGI("%s", "无法打开解码器");
        return;
    }

    //压缩数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
    // TODO 输出采样率=输入采样率 这样能保证声音解码后的正确性
    int out_sample_rate = in_sample_rate;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = codecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx,
                       out_ch_layout, out_sample_fmt, out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sample_rate,
                       0, NULL);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end

    /*// JNI begin------------------
    // JasonPlayer
    jclass player_class = (*env)->GetObjectClass(env,jthiz);
    // AudioTrack对象
    jmethodID create_audio_track_mid = (*env)->GetMethodID(env,player_class,"createAudioTrack","(II)Landroid/media/AudioTrack;");
    jobject audio_track = (*env)->CallObjectMethod(env,jthiz,create_audio_track_mid,out_sample_rate,out_channel_nb);
    //调用AudioTrack.play方法
    jclass audio_track_class = (*env)->GetObjectClass(env,audio_track);
    jmethodID audio_track_play_mid = (*env)->GetMethodID(env,audio_track_class,"play","()V");
    (*env)->CallVoidMethod(env,audio_track,audio_track_play_mid);
    // AudioTrack.write
    jmethodID audio_track_write_mid = (*env)->GetMethodID(env,audio_track_class,"write","([BII)I");
    //JNI end------------------*/

    // TODO JNI调用 begin----------------
    // AudioTrack 对象
    jmethodID create_audio_track_mid = env->GetStaticMethodID(
            type, "createAudioTrack", "(II)Landroid/media/AudioTrack;");
    jobject audio_track = env->CallStaticObjectMethod(
            type, create_audio_track_mid, out_sample_rate, out_channel_nb);
    // 调用 AudioTrac.paly 方法
    jclass audio_track_class = env->GetObjectClass(audio_track);
    jmethodID audio_track_play_mid = env->GetMethodID(audio_track_class, "play", "()V");
    env->CallVoidMethod(audio_track, audio_track_play_mid);
    // 调用 AudioTrack.write 方法
    jmethodID audio_track_write_mid = env->GetMethodID(audio_track_class, "write", "([BII)I");
    // TODO JNI调用 end----------------

    //16bit 44100 PCM 数据 【缓冲区再大一点也是可以的，小了就不行】
    //uint8_t *out_buffer = (uint8_t *) av_malloc(2*44100);
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

    int got_frame = 0, index = 0, ret;
    //不断读取压缩数据
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        // TODO 解码音频类型的 Packet
        if (packet->stream_index == audio_stream_idx) {
            //解码
            ret = avcodec_decode_audio4(codecCtx, frame, &got_frame, packet);

            if (ret < 0) {
                LOGI("%s", "解码完成");
            }
            //解码一帧成功
            if (got_frame > 0) {
                LOGI("解码：%d", index++);
                swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,
                            (const uint8_t **) frame->data, frame->nb_samples);
                //获取sample的size
                int out_buffer_size = av_samples_get_buffer_size(
                        NULL, out_channel_nb,
                        frame->nb_samples, out_sample_fmt, 1);

                // out_buffer缓冲区数据，转成 byte数组
                jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
                // 通过操作 sample_byte_p就能改变 audio_sample_array中的数据
                jbyte *sample_byte_p = env->GetByteArrayElements(audio_sample_array, NULL);
                // out_buffer的数据复制到 sample_byte_p
                memcpy(sample_byte_p, out_buffer, out_buffer_size);
                // 同步
                env->ReleaseByteArrayElements(audio_sample_array, sample_byte_p, 0);
                // TODO AudioTrack.write PCM数据
                env->CallIntMethod(audio_track, audio_track_write_mid,
                                   audio_sample_array, 0, out_buffer_size);

                // JNI ERROR:local reference table overflow 【内存溢出】
                // 上面不断地创建数组 audio_sample_array，特别占用内存，要及时进行释放
                // 释放局部引用
                env->DeleteLocalRef(audio_sample_array);

                usleep(1000 * 16);
            }
        }

        av_free_packet(packet);
    }

    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrCtx);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
    env->ReleaseStringUTFChars(output_jstr, output_cstr);
}

