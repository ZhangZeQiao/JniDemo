//
// Created by joe on 2017/9/14.
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

extern "C" {
// 封装格式处理
#include "include/libavformat/avformat.h"
// 解码
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>
}

// player练习代码
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_playerDecode
        (JNIEnv *env, jclass type, jstring input_jstr, jstring output_jstr) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, 0);
    const char *output_cstr = env->GetStringUTFChars(output_jstr, 0);

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
    int video_stream_idx = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; ++i) {
        // 根据类型判断，是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    // 4、获取视频解码器
    AVCodecContext *pCodecContext = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "无法解码");
        return;
    }

    // 5、解码之前要先打开解码器
    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        LOGE("%s", "无法打开解码器");
        return;
    }

    // 编码/压缩数据的初始化
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket)); // 一定要先手动分配内存
    av_init_packet(pPacket);
    // 像素数据/解码数据
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();
    // 只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    // 缓冲区分配内存
    uint8_t *out_buffer =
            (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P,
                                                     pCodecContext->width,
                                                     pCodecContext->height));
    // 设置 yuvFrame的缓冲区、像素格式
    avpicture_fill((AVPicture *) yuvFrame, out_buffer, AV_PIX_FMT_YUV420P,
                   pCodecContext->width, pCodecContext->height);

    // 打开输出文件
    FILE *fp_yuv = fopen(output_cstr, "wb");

    // 用于像素格式转换或者缩放
    struct SwsContext *sws_ctx = sws_getContext(
            // 原画面宽高
            pCodecContext->width, pCodecContext->height,
            // 原画面像素格式
            pCodecContext->pix_fmt,
            // 目标画面/缩放后宽高
            pCodecContext->width, pCodecContext->height,
            // 目标画面像素格式
            AV_PIX_FMT_YUV420P,
            // 算法
            SWS_BILINEAR,
            NULL, NULL, NULL);

    // 6、一帧一帧读取压缩的视频数据 AVPacket
    int len, got_picture_ptr, framecount = 0;
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        // 解码AVPacket->AVFrame
        len = avcodec_decode_video2(pCodecContext, pFrame, &got_picture_ptr, pPacket);
        // @param[in,out] got_picture_ptr Zero if no frame could be decompressed, otherwise, it is nonzero.
        // 0代表解码完成，非0代表正在解码
        if (got_picture_ptr) {
            // pframe->yuvFrame (YUV420P)
            // 转为指定的YUV420P像素帧
            sws_scale(sws_ctx,
                      (const uint8_t *const *) pFrame->data,
                      pFrame->linesize, 0, pFrame->height,
                      yuvFrame->data, yuvFrame->linesize);

            // 向YUV文件保存解码之后的帧数据
            // AVFrame->YUV
            // 一个像素包含一个Y
            int y_size = pCodecContext->width * pCodecContext->height;
            fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
            // Y是U、V的四倍
            fwrite(yuvFrame->data[1], 1, y_size / 4, fp_yuv);
            fwrite(yuvFrame->data[2], 1, y_size / 4, fp_yuv);

            LOGI("解码%d帧", framecount++);
        }
        // 释放资源
        av_free_packet(pPacket);
        // av_packet_free(&pPacket);
    }

    // 释放资源
    fclose(fp_yuv);
    av_frame_free(&pFrame);
    // 关闭解码器
    avcodec_close(pCodecContext);
    avformat_free_context(pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
    env->ReleaseStringUTFChars(output_jstr, output_cstr);
}

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

// TODO --------------------------------------------------------------------------------------------
// 基于上面的 player练习代码
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_render(
        JNIEnv *env, jclass type, jstring input_jstr, jobject surface) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, 0);

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
    int video_stream_idx = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; ++i) {
        // 根据类型判断，是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    // 4、获取视频解码器
    AVCodecContext *pCodecContext = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "无法解码");
        return;
    }

    // 5、解码之前要先打开解码器
    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
        LOGE("%s", "无法打开解码器");
        return;
    }

    // 编码/压缩数据的初始化
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket)); // 一定要先手动分配内存

    // 像素数据/解码数据
    AVFrame *yuvFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();

    // native绘制
    // 窗体
    ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(env, surface);
    // 绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    // 6、一帧一帧读取压缩的视频数据 AVPacket
    int len, got_picture_ptr, framecount = 0;
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        // TODO 解码视频类型的 Packet
        if (pPacket->stream_index == video_stream_idx) {

            // 解码AVPacket->AVFrame
            len = avcodec_decode_video2(pCodecContext, yuvFrame, &got_picture_ptr, pPacket);
            // @param[in,out] got_picture_ptr Zero if no frame could be decompressed, otherwise, it is nonzero.
            // 0代表解码完成，非0代表正在解码
            if (got_picture_ptr) {
                LOGI("解码%d帧", framecount++);

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

                // TODO 这里是用上面解码成 YUV进行 RGB的转换，但是视频解码的最终结果不一定是 YUV，这时可能会出现播放时花屏
                // TODO 解决方法是进行判断，用不同的函数进行转换

                // YUV->RGBA_8888
                // TODO 直接 I420ToARGB 找不到，用下面的方法就能找到，是不是很机智 ...
                // TODO 可能是 C中能找到，C++要用下面的方法才能找到
                // TODO 查看源码 namespace libyuv {}
                libyuv::I420ToABGR(yuvFrame->data[0], yuvFrame->linesize[0], // y的数据跟一行大小
                                   yuvFrame->data[1], yuvFrame->linesize[1], // u的数据跟一行大小
                                   yuvFrame->data[2], yuvFrame->linesize[2], // v的数据跟一行大小
                        // yuvFrame->data[2], yuvFrame->linesize[2], // v的数据跟一行大小
                        // yuvFrame->data[1], yuvFrame->linesize[1], // u的数据跟一行大小
                                   rgbFrame->data[0], rgbFrame->linesize[0], // 指定 rgb
                                   pCodecContext->width, pCodecContext->height); // 宽高

                // unlock 解锁
                ANativeWindow_unlockAndPost(aNativeWindow);

                // TODO 不断解码不断绘制，频率太高，休眠一下
                // TODO 不然会报错：Fatal signal 11 (SIGSEGV),code 1,fault addr 0x0 in tid
                usleep(1000 * 16);
            }
        }
        // 释放资源
        av_free_packet(pPacket);
        // av_packet_free(&pPacket);
    }

    // 释放资源
    ANativeWindow_release(aNativeWindow);
    av_frame_free(&yuvFrame);
    // 关闭解码器
    avcodec_close(pCodecContext);
    avformat_free_context(pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
}
// TODO --------------------------------------------------------------------------------------------
// TODO 音视频同步播放最终代码【同步、封装】

// nb_streams 视频文件中存在：视频流0，音频流1，字幕2
// #define MAX_STREAM 3
#define MAX_STREAM 2

struct Player {
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
 * 解码视频
 */
void decode_video(struct Player *player, AVPacket *pPacket) {
    AVCodecContext *pCodecContext = player->input_codec_ctx[player->video_stream_index];
    ANativeWindow *aNativeWindow = player->nativeWindow;

    // 像素数据/解码数据
    AVFrame *yuvFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();

    // 绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    int len, got_picture_ptr = 0;

    // 解码AVPacket->AVFrame
    len = avcodec_decode_video2(pCodecContext, yuvFrame, &got_picture_ptr, pPacket);
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
                           yuvFrame->data[1], yuvFrame->linesize[1], // u的数据跟一行大小
                           yuvFrame->data[2], yuvFrame->linesize[2], // v的数据跟一行大小
                           rgbFrame->data[0], rgbFrame->linesize[0], // 指定 rgb
                           pCodecContext->width, pCodecContext->height); // 宽高

        // unlock 解锁
        ANativeWindow_unlockAndPost(aNativeWindow);

        usleep(1000 * 16);
    }

    av_frame_free(&yuvFrame);
    av_frame_free(&rgbFrame);
}

/**
 * 解码子线程函数
 */
void *decode_data(void *arg) {
    struct Player *player = (struct Player *) arg;

    // 编码/压缩数据的初始化
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket)); // 一定要先手动分配内存

    // 6、一帧一帧读取压缩的视频数据 AVPacket
    int video_frame_count = 0;
    while (av_read_frame(player->input_format_ctx, pPacket) >= 0) {
        if (pPacket->stream_index == player->video_stream_index) {
            decode_video(player, pPacket);
            LOGI("video_frame_count:%d", video_frame_count++);
        }
        // 释放资源
        av_free_packet(pPacket);
    }
}

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

    // TODO 1 初始化封装格式上下文
    init_input_format_ctx(player, input_cstr);

    // TODO 2 获取音视频解码器，并打开
    int video_stream_index = player->video_stream_index;
    int audio_stream_index = player->audio_stream_index;
    init_codec_ctx(player, video_stream_index);
    init_codec_ctx(player, audio_stream_index);

    // TODO native绘制窗体初始化
    decode_video_prepare(env, player, surface);

    // TODO 3 在子线程中进行解码
    pthread_create(&(player->decode_threads[video_stream_index]), NULL, decode_data,
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
