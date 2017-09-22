//
// Created by joe on 2017/9/22.
//

#include <jni.h>

// TODO 这个要放最前面才有用
#ifndef _Nonnull
#define _Nonnull

#include <pthread.h>

#endif

#include "video_utils.h"

#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"xq",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"xq",FORMAT,##__VA_ARGS__);

// TODO 引入 native原生绘制的头文件
// TODO 用这个要在 CMakeLists.txt中的 target_link_libraries配置 -landroid、-ljnigraphics
#include <android/native_window_jni.h>
#include <android/native_window.h>
// C++代码混编
extern "C" {
// TODO 引入YUV与RGB之间相互转换、旋转、缩放的库
#include "libyuv.h"
}

#include <unistd.h>

extern "C" {
// 封装格式处理
#include "include/libavformat/avformat.h"
// 解码
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>

//重采样
#include "libswresample/swresample.h"
}

// TODO --------------------------------------------------------------------------------------------
// TODO 音视频同步播放【抽取、封装】
/*
// nb_streams 视频文件中存在：视频流0，音频流1，字幕2
// #define MAX_STREAM 3
#define MAX_STREAM 2
// 缓冲区
#define MAX_AUDIO_FRME_SIZE 48000 * 4

struct Player {
    JavaVM *javaVM;

    // 封装格式上下文
    AVFormatContext *input_format_ctx;
    // 音频和视频流的索引位置
    int video_stream_index;
    int audio_stream_index;
    // 解码器上下文数组
    AVCodecContext *input_codec_ctx[MAX_STREAM];
    // 解码线程 ID
    pthread_t decode_threads[MAX_STREAM];
    ANativeWindow *nativeWindow;

    SwrContext *swr_ctx;
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt;
    //输入采样率
    int in_sample_rate;
    //输出采样率
    int out_sample_rate;
    //输出的声道个数
    int out_channel_nb;
    //JNI
    jobject audio_track;
    jmethodID audio_track_write_mid;
};

*//**
 * 初始化封装格式上下文，获取音频视频流的索引位置
 *//*
void init_input_format_ctx(struct Player *player, const char *input_cstr) {
    // 1、注册组件
    av_register_all();

    // 封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    // 2、打开输入视频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }

    // 3、获取视频信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
        return;
    }

    // 视频解码，需要找到视频对应的AVStream在 pFormatCtx->stream 的索引位置
    // 获取音频和视频流的索引位置
    int i = 0;
    for (; i < pFormatCtx->nb_streams; ++i) {
        // 根据类型判断，是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            player->video_stream_index = i;
        } else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            player->audio_stream_index = i;
        }
    }

    player->input_format_ctx = pFormatCtx;
}

*//**
 * 初始化解码器上下文
 *//*
void init_codec_ctx(struct Player *player, int stream_idx) {
    AVFormatContext *format_ctx = player->input_format_ctx;
    // 4、获取解码器
    AVCodecContext *pCodec_ctx = format_ctx->streams[stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodec_ctx->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "无法解码");
        return;
    }

    // 5、解码之前要先打开解码器
    if (avcodec_open2(pCodec_ctx, pCodec, NULL) < 0) {
        LOGE("%s", "无法打开解码器");
        return;
    }

    player->input_codec_ctx[stream_idx] = pCodec_ctx;
}

*//**
 * 视频解码
 *//*
void decode_video(struct Player *player, AVPacket *pPacket) {
    AVCodecContext *pCodecContext = player->input_codec_ctx[player->video_stream_index];
    ANativeWindow *aNativeWindow = player->nativeWindow;

    // 像素数据/解码数据
    AVFrame *yuvFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();

    // 绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    int got_picture_ptr = 0;

    // 解码AVPacket->AVFrame
    avcodec_decode_video2(pCodecContext, yuvFrame, &got_picture_ptr, pPacket);
    // @param[in,out] got_picture_ptr Zero if no frame could be decompressed, otherwise, it is nonzero.
    // 0代表解码完成，非0代表正在解码
    if (got_picture_ptr) {
        // lock 锁定
        // 设置缓冲区的属性（宽、高、像素格式）
        ANativeWindow_setBuffersGeometry(aNativeWindow,
                                         pCodecContext->width,
                                         pCodecContext->height,
                // 用的格式要和 SurfaceHolder设置的一致
                                         AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM); // WINDOW_FORMAT_RGBA_8888
        ANativeWindow_lock(aNativeWindow, &outBuffer, NULL);

        // 设置 yuvFrame的属性（像素格式、宽高）和缓冲区
        // rgbFrame缓冲区与 outBuffer.bits是同一块内存
        avpicture_fill((AVPicture *) rgbFrame,
                       (const uint8_t *) outBuffer.bits, // 共用缓冲区
                       AV_PIX_FMT_RGBA,
                       pCodecContext->width, pCodecContext->height);

        // YUV->RGBA_8888
        libyuv::I420ToABGR(yuvFrame->data[0], yuvFrame->linesize[0], // y的数据跟一行大小
                           yuvFrame->data[2], yuvFrame->linesize[2], // v的数据跟一行大小
                           yuvFrame->data[1], yuvFrame->linesize[1], // u的数据跟一行大小
                           rgbFrame->data[0], rgbFrame->linesize[0], // 指定 rgb
                           pCodecContext->width, pCodecContext->height); // 宽高

        // unlock 解锁
        ANativeWindow_unlockAndPost(aNativeWindow);

        usleep(1000 * 16);
    }

    av_frame_free(&yuvFrame);
    av_frame_free(&rgbFrame);
}

void jni_audio_prepare(JNIEnv *env, jclass type, struct Player *player) {
    // AudioTrack 对象
    jmethodID create_audio_track_mid = env->GetStaticMethodID(
            type, "createAudioTrack", "(II)Landroid/media/AudioTrack;");
    jobject audio_track = env->CallStaticObjectMethod(
            type, create_audio_track_mid, player->out_sample_rate, player->out_channel_nb);
    // 调用 AudioTrac.paly 方法
    jclass audio_track_class = env->GetObjectClass(audio_track);
    jmethodID audio_track_play_mid = env->GetMethodID(audio_track_class, "play", "()V");
    env->CallVoidMethod(audio_track, audio_track_play_mid);
    // 调用 AudioTrack.write 方法
    jmethodID audio_track_write_mid = env->GetMethodID(audio_track_class, "write", "([BII)I");

    // 注意子线程获取不到 Java方法，要在主线程获取，并变成全局的
    player->audio_track = env->NewGlobalRef(audio_track);
    //(*env)->DeleteGlobalRef
    player->audio_track_write_mid = audio_track_write_mid;
}

*//**
 * 音频解码准备
 *//*
void decode_audio_prepare(struct Player *player) {
    AVCodecContext *codecCtx = player->input_codec_ctx[player->audio_stream_index];

    //frame->16bit 44100 PCM 统一音频采样格式与采样率【用于重采样】
    SwrContext *swrCtx = swr_alloc();
    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
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

    player->in_sample_fmt = in_sample_fmt;
    player->out_sample_fmt = out_sample_fmt;
    player->in_sample_rate = in_sample_rate;
    player->out_sample_rate = out_sample_rate;
    player->out_channel_nb = out_channel_nb;
    player->swr_ctx = swrCtx;
}

*//**
 * 音频解码
 *//*
void decode_audio(struct Player *player, AVPacket *packet) {
    AVCodecContext *codecCtx = player->input_codec_ctx[player->audio_stream_index];

    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    int got_frame;
    avcodec_decode_audio4(codecCtx, frame, &got_frame, packet);
    //uint8_t *out_buffer = (uint8_t *) av_malloc(2*44100);【重采样缓冲区】
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);
    //解码一帧成功
    if (got_frame > 0) {
        swr_convert(player->swr_ctx, &out_buffer, MAX_AUDIO_FRME_SIZE,
                    (const uint8_t **) frame->data, frame->nb_samples);
        //获取sample的size
        int out_buffer_size = av_samples_get_buffer_size(
                NULL, player->out_channel_nb,
                frame->nb_samples, player->out_sample_fmt, 1);

        // 关联当前线程的 JNIEnv
        JavaVM *javaVM = player->javaVM;
        JNIEnv *env = NULL;
        javaVM->AttachCurrentThread(&env, NULL);

        // out_buffer缓冲区数据，转成 byte数组
        jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
        // 通过操作 sample_byte_p就能改变 audio_sample_array中的数据
        jbyte *sample_byte_p = env->GetByteArrayElements(audio_sample_array, NULL);
        // out_buffer的数据复制到 sample_byte_p
        memcpy(sample_byte_p, out_buffer, out_buffer_size);
        // 同步
        env->ReleaseByteArrayElements(audio_sample_array, sample_byte_p, 0);
        env->CallIntMethod(player->audio_track, player->audio_track_write_mid,
                           audio_sample_array, 0, out_buffer_size);

        // JNI ERROR:local reference table overflow 【内存溢出】
        // 上面不断地创建数组 audio_sample_array，特别占用内存，要及时进行释放
        // 释放局部引用
        env->DeleteLocalRef(audio_sample_array);
        javaVM->DetachCurrentThread();
        usleep(1000 * 16);
    }

    av_frame_free(&frame);
}

*//**
 * 解码子线程函数
 *//*
void *decode_data(void *arg) {
    struct Player *player = (struct Player *) arg;
    AVFormatContext *format_ctx = player->input_format_ctx;

    // 编码/压缩数据的初始化
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket)); // 一定要先手动分配内存

    // 6、一帧一帧读取压缩的视频数据 AVPacket
    int video_frame_count = 0;
    while (av_read_frame(format_ctx, pPacket) >= 0) {
        if (pPacket->stream_index == player->video_stream_index) {
            //decode_video(player, pPacket);
            LOGI("video_frame_count ------ %d", video_frame_count++);
        } else if (pPacket->stream_index == player->audio_stream_index) {
            decode_audio(player, pPacket);
            LOGI("audio_frame_count ~~~~~~ %d", video_frame_count++);
        }
        // 释放资源
        av_free_packet(pPacket);
    }
}

*//**
 * 视频解码准备
 *//*
void decode_video_prepare(JNIEnv *env, struct Player *player, jobject surface) {
    // native绘制
    // 窗体
    player->nativeWindow = ANativeWindow_fromSurface(env, surface);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_play(
        JNIEnv *env, jclass type, jstring input_jstr, jobject surface) {

    const char *input_cstr = env->GetStringUTFChars(input_jstr, 0);
    struct Player *player = (Player *) malloc(sizeof(struct Player));

    env->GetJavaVM(&(player->javaVM));

// 1 初始化封装格式上下文
    init_input_format_ctx(player, input_cstr);

// 2 获取音视频解码器，并打开
    int video_stream_index = player->video_stream_index;
    int audio_stream_index = player->audio_stream_index;
    init_codec_ctx(player, video_stream_index);
    init_codec_ctx(player, audio_stream_index);

// 初始化
    decode_video_prepare(env, player, surface);
    decode_audio_prepare(player);

    jni_audio_prepare(env, type, player);

// 3 在子线程中进行解码
// pthread_create(&(player->decode_threads[video_stream_index]), NULL, decode_data, (void *) player);
    pthread_create(&(player->decode_threads[audio_stream_index]), NULL, decode_data,
                   (void *) player);

// 子线程结束之前不能释放
// 释放资源
*//*ANativeWindow_release(aNativeWindow);
av_frame_free(&yuvFrame);
// 关闭解码器
avcodec_close(pCodecContext);
avformat_free_context(pFormatCtx);

free(player);
env->ReleaseStringUTFChars(input_jstr, input_cstr);*//*
}
*/

// TODO --------------------------------------------------------------------------------------------
// TODO 音视频同步播放最终代码【同步】


// nb_streams 视频文件中存在：视频流0，音频流1，字幕2
// #define MAX_STREAM 3
#define MAX_STREAM 2
// 缓冲区
#define MAX_AUDIO_FRME_SIZE 48000 * 4

struct Player {
    JavaVM *javaVM;

    // 封装格式上下文
    AVFormatContext *input_format_ctx;
    // 音频和视频流的索引位置
    int video_stream_index;
    int audio_stream_index;
    // 解码器上下文数组
    AVCodecContext *input_codec_ctx[MAX_STREAM];
    // 解码线程 ID
    pthread_t decode_threads[MAX_STREAM];
    ANativeWindow *nativeWindow;

    SwrContext *swr_ctx;
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt;
    //输入采样率
    int in_sample_rate;
    //输出采样率
    int out_sample_rate;
    //输出的声道个数
    int out_channel_nb;
    //JNI
    jobject audio_track;
    jmethodID audio_track_write_mid;
};

/**
 * 初始化封装格式上下文，获取音频视频流的索引位置
 */
void init_input_format_ctx(struct Player *player, const char *input_cstr) {
    // 1、注册组件
    av_register_all();

    // 封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    // 2、打开输入视频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }

    // 3、获取视频信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
        return;
    }

    // 视频解码，需要找到视频对应的AVStream在 pFormatCtx->stream 的索引位置
    // 获取音频和视频流的索引位置
    int i = 0;
    for (; i < pFormatCtx->nb_streams; ++i) {
        // 根据类型判断，是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            player->video_stream_index = i;
        } else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            player->audio_stream_index = i;
        }
    }

    player->input_format_ctx = pFormatCtx;
}

/**
 * 初始化解码器上下文
 */
void init_codec_ctx(struct Player *player, int stream_idx) {
    AVFormatContext *format_ctx = player->input_format_ctx;
    // 4、获取解码器
    AVCodecContext *pCodec_ctx = format_ctx->streams[stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodec_ctx->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "无法解码");
        return;
    }

    // 5、解码之前要先打开解码器
    if (avcodec_open2(pCodec_ctx, pCodec, NULL) < 0) {
        LOGE("%s", "无法打开解码器");
        return;
    }

    player->input_codec_ctx[stream_idx] = pCodec_ctx;
}

/**
 * 视频解码
 */
void decode_video(struct Player *player, AVPacket *pPacket) {
    AVCodecContext *pCodecContext = player->input_codec_ctx[player->video_stream_index];
    ANativeWindow *aNativeWindow = player->nativeWindow;

    // 像素数据/解码数据
    AVFrame *yuvFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();

    // 绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    int got_picture_ptr = 0;

    // 解码AVPacket->AVFrame
    avcodec_decode_video2(pCodecContext, yuvFrame, &got_picture_ptr, pPacket);
    // @param[in,out] got_picture_ptr Zero if no frame could be decompressed, otherwise, it is nonzero.
    // 0代表解码完成，非0代表正在解码
    if (got_picture_ptr) {
        // lock 锁定
        // 设置缓冲区的属性（宽、高、像素格式）
        ANativeWindow_setBuffersGeometry(aNativeWindow,
                                         pCodecContext->width,
                                         pCodecContext->height,
                // 用的格式要和 SurfaceHolder设置的一致
                                         AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM); // WINDOW_FORMAT_RGBA_8888
        ANativeWindow_lock(aNativeWindow, &outBuffer, NULL);

        // 设置 yuvFrame的属性（像素格式、宽高）和缓冲区
        // rgbFrame缓冲区与 outBuffer.bits是同一块内存
        avpicture_fill((AVPicture *) rgbFrame,
                       (const uint8_t *) outBuffer.bits, // 共用缓冲区
                       AV_PIX_FMT_RGBA,
                       pCodecContext->width, pCodecContext->height);

        // YUV->RGBA_8888
        libyuv::I420ToABGR(yuvFrame->data[0], yuvFrame->linesize[0], // y的数据跟一行大小
                           yuvFrame->data[2], yuvFrame->linesize[2], // v的数据跟一行大小
                           yuvFrame->data[1], yuvFrame->linesize[1], // u的数据跟一行大小
                           rgbFrame->data[0], rgbFrame->linesize[0], // 指定 rgb
                           pCodecContext->width, pCodecContext->height); // 宽高

        // unlock 解锁
        ANativeWindow_unlockAndPost(aNativeWindow);

        usleep(1000 * 16);
    }

    av_frame_free(&yuvFrame);
    av_frame_free(&rgbFrame);
}

void jni_audio_prepare(JNIEnv *env, jclass type, struct Player *player) {
    // AudioTrack 对象
    jmethodID create_audio_track_mid = env->GetStaticMethodID(
            type, "createAudioTrack", "(II)Landroid/media/AudioTrack;");
    jobject audio_track = env->CallStaticObjectMethod(
            type, create_audio_track_mid, player->out_sample_rate, player->out_channel_nb);
    // 调用 AudioTrac.paly 方法
    jclass audio_track_class = env->GetObjectClass(audio_track);
    jmethodID audio_track_play_mid = env->GetMethodID(audio_track_class, "play", "()V");
    env->CallVoidMethod(audio_track, audio_track_play_mid);
    // 调用 AudioTrack.write 方法
    jmethodID audio_track_write_mid = env->GetMethodID(audio_track_class, "write", "([BII)I");

    // TODO 注意子线程获取不到 Java方法，要在主线程获取，并变成全局的
    player->audio_track = env->NewGlobalRef(audio_track);
    //(*env)->DeleteGlobalRef
    player->audio_track_write_mid = audio_track_write_mid;
}

/**
 * 音频解码准备
 */
void decode_audio_prepare(struct Player *player) {
    AVCodecContext *codecCtx = player->input_codec_ctx[player->audio_stream_index];

    //frame->16bit 44100 PCM 统一音频采样格式与采样率【用于重采样】
    SwrContext *swrCtx = swr_alloc();
    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
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

    player->in_sample_fmt = in_sample_fmt;
    player->out_sample_fmt = out_sample_fmt;
    player->in_sample_rate = in_sample_rate;
    player->out_sample_rate = out_sample_rate;
    player->out_channel_nb = out_channel_nb;
    player->swr_ctx = swrCtx;
}

/**
 * 音频解码
 */
void decode_audio(struct Player *player, AVPacket *packet) {
    AVCodecContext *codecCtx = player->input_codec_ctx[player->audio_stream_index];

    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    int got_frame;
    avcodec_decode_audio4(codecCtx, frame, &got_frame, packet);
    //uint8_t *out_buffer = (uint8_t *) av_malloc(2*44100);【重采样缓冲区】
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);
    //解码一帧成功
    if (got_frame > 0) {
        swr_convert(player->swr_ctx, &out_buffer, MAX_AUDIO_FRME_SIZE,
                    (const uint8_t **) frame->data, frame->nb_samples);
        //获取sample的size
        int out_buffer_size = av_samples_get_buffer_size(
                NULL, player->out_channel_nb,
                frame->nb_samples, player->out_sample_fmt, 1);

        // TODO 关联当前线程的 JNIEnv
        JavaVM *javaVM = player->javaVM;
        JNIEnv *env = NULL;
        javaVM->AttachCurrentThread(&env, NULL);

        // out_buffer缓冲区数据，转成 byte数组
        jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
        // 通过操作 sample_byte_p就能改变 audio_sample_array中的数据
        jbyte *sample_byte_p = env->GetByteArrayElements(audio_sample_array, NULL);
        // out_buffer的数据复制到 sample_byte_p
        memcpy(sample_byte_p, out_buffer, out_buffer_size);
        // 同步
        env->ReleaseByteArrayElements(audio_sample_array, sample_byte_p, 0);
        env->CallIntMethod(player->audio_track, player->audio_track_write_mid,
                           audio_sample_array, 0, out_buffer_size);

        // JNI ERROR:local reference table overflow 【内存溢出】
        // 上面不断地创建数组 audio_sample_array，特别占用内存，要及时进行释放
        // 释放局部引用
        env->DeleteLocalRef(audio_sample_array);
        javaVM->DetachCurrentThread();
        usleep(1000 * 16);
    }

    av_frame_free(&frame);
}

/**
 * 解码子线程函数
 */
void *decode_data(void *arg) {
    struct Player *player = (struct Player *) arg;
    AVFormatContext *format_ctx = player->input_format_ctx;

    // 编码/压缩数据的初始化
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket)); // 一定要先手动分配内存

    // 6、一帧一帧读取压缩的视频数据 AVPacket
    int video_frame_count = 0;
    while (av_read_frame(format_ctx, pPacket) >= 0) {
        if (pPacket->stream_index == player->video_stream_index) {
            //decode_video(player, pPacket);
            LOGI("video_frame_count ------ %d", video_frame_count++);
        } else if (pPacket->stream_index == player->audio_stream_index) {
            decode_audio(player, pPacket);
            LOGI("audio_frame_count ~~~~~~ %d", video_frame_count++);
        }
        // 释放资源
        av_free_packet(pPacket);
    }
}

/**
 * 视频解码准备
 */
void decode_video_prepare(JNIEnv *env, struct Player *player, jobject surface) {
    // native绘制
    // 窗体
    player->nativeWindow = ANativeWindow_fromSurface(env, surface);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_play(
        JNIEnv *env, jclass type, jstring input_jstr, jobject surface) {

    const char *input_cstr = env->GetStringUTFChars(input_jstr, 0);
    struct Player *player = (Player *) malloc(sizeof(struct Player));

    env->GetJavaVM(&(player->javaVM));

    // TODO 1 初始化封装格式上下文
    init_input_format_ctx(player, input_cstr);

    // TODO 2 获取音视频解码器，并打开
    int video_stream_index = player->video_stream_index;
    int audio_stream_index = player->audio_stream_index;
    init_codec_ctx(player, video_stream_index);
    init_codec_ctx(player, audio_stream_index);

    // TODO 初始化
    decode_video_prepare(env, player, surface);
    decode_audio_prepare(player);

    jni_audio_prepare(env, type, player);

    // TODO 3 在子线程中进行解码
    // pthread_create(&(player->decode_threads[video_stream_index]), NULL, decode_data, (void *) player);
    pthread_create(&(player->decode_threads[audio_stream_index]), NULL, decode_data,
                   (void *) player);

    // TODO 子线程结束之前不能释放
    // 释放资源
    /*ANativeWindow_release(aNativeWindow);
    av_frame_free(&yuvFrame);
    // 关闭解码器
    avcodec_close(pCodecContext);
    avformat_free_context(pFormatCtx);

    free(player);
    env->ReleaseStringUTFChars(input_jstr, input_cstr);*/
}
