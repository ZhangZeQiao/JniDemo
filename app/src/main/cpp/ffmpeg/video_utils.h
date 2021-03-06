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
Java_com_xq_jnidemo_ffmpeg_VideoUtils_playerDecode(JNIEnv *, jclass, jstring, jstring);

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_render(JNIEnv *, jclass, jstring, jobject);

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_soundDecode(JNIEnv *, jclass, jstring, jstring);

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_soundPlay(JNIEnv *, jclass, jstring, jstring);

JNIEXPORT void JNICALL
Java_com_xq_jnidemo_ffmpeg_VideoUtils_play(JNIEnv *, jclass, jstring, jobject);

#ifdef __cplusplus
}
#endif
#endif
