//
// Created by joe on 2017/9/14.
//

#include <jni.h>

#ifndef JNIDEMO_VIDEO_UTILS_H
#define JNIDEMO_VIDEO_UTILS_H
#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_decode(JNIEnv *, jclass, jstring, jstring);

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_player(JNIEnv *, jclass, jstring, jstring);

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_render(JNIEnv *, jclass, jstring, jobject);

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_sound(JNIEnv *, jclass, jstring, jstring);

#ifdef __cplusplus
}
#endif
#endif
