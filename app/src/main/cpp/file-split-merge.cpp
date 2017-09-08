//
// Created by joe on 2017/9/8.
//

#include <jni.h>
#include <string>
#include <stdlib.h>
#include <android/log.h> // TODO ：jni中使用 log

// TODO ：文件拆分
// TODO ：jni中使用 log，宏定义函数
#define LOGV(FORMAT, ...) __android_log_print(ANDROID_LOG_VERBOSE, "zzq", FORMAT, __VA_ARGS__);

// 获取文件大小
long get_file_size(const char *path) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_file_1split_1merge_FileSplitMergeActivity_split
        (JNIEnv *env, jobject instance, jstring path_, jstring path_part_, jint count) {
    const char *path = env->GetStringUTFChars(path_, 0);
    const char *path_part = env->GetStringUTFChars(path_part_, 0);

    // jstring要转成字符指针

    // 得到分割后的子文件路径列表
    char **patches = (char **) malloc(sizeof(char *) * count);
    int i = 0;
    for (; i < count; ++i) {
        patches[i] = (char *) malloc(sizeof(char) * 100);
        // 元素赋值
        sprintf(patches[i], path_part, (i + 1));
        LOGV("patch path : %s", patches[i]);
    }

    // 不断读取path文件，循环写入 count个文件中
    int filesize = get_file_size(path);
    FILE *fpr = fopen(path, "rb");
    // 整除
    if (filesize % count == 0) {
        // 单个文件的大小
        int part = filesize / count;
        // 逐一写入不同的分割子文件中
        int var = 0;
        for (; var < count; ++var) {
            FILE *fpw = fopen(patches[var], "wb");
            int j = 0;
            for (; j < part; ++j) {
                // 边读边写
                fputc(fgetc(fpr), fpw);
            }
            fclose(fpw);
        }
    } else {
        // 不能整除
        // 单个文件的大小
        int part = filesize / (count - 1);
        // 逐一写入不同的分割子文件中
        int var = 0;
        for (; var < count - 1; ++var) {
            FILE *fpw = fopen(patches[var], "wb");
            int j = 0;
            for (; j < part; ++j) {
                // 边读边写
                fputc(fgetc(fpr), fpw);
            }
            fclose(fpw);
        }
        // 最后一个
        FILE *fpw = fopen(patches[count - 1], "wb");
        int i = 0;
        for (; i < filesize % (count - 1); ++i) {
            fputc(fgetc(fpr), fpw);
        }
        fclose(fpw);
    }
    // 关闭被分割的文件
    fclose(fpr);

    // 释放
    i = 0;
    for (; i < count; ++i) {
        free(patches[i]);
    }
    free(patches);

    env->ReleaseStringUTFChars(path_, path);
    env->ReleaseStringUTFChars(path_part_, path_part);
}
// TODO ：文件合并
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_file_1split_1merge_FileSplitMergeActivity_merge
        (JNIEnv *env, jobject instance, jstring path_part_, jint count, jstring merge_path_) {
    const char *path_part = env->GetStringUTFChars(path_part_, 0);
    const char *merge_path = env->GetStringUTFChars(merge_path_, 0);

    // 得到分割后的子文件路径列表
    char **patches = (char **) malloc(sizeof(char *) * count);
    int i = 0;
    for (; i < count; ++i) {
        patches[i] = (char *) malloc(sizeof(char) * 100);
        // 元素赋值
        sprintf(patches[i], path_part, (i + 1));
        LOGV("patch path : %s", patches[i]);
    }

    FILE *fpw = fopen(merge_path, "wb");
    // 把所有的分割文件读取一遍，写入一个总的文件中
    i = 0;
    for (; i < count; ++i) {
        // 每个文件的大小
        int filesize = get_file_size(patches[i]);
        FILE *fpr = fopen(patches[i], "rb");
        int j = 0;
        for (; j < filesize; ++j) {
            fputc(fgetc(fpr), fpw);
        }
        fclose(fpr);
    }
    fclose(fpw);

    // 释放
    i = 0;
    for (; i < count; ++i) {
        free(patches[i]);
    }
    free(patches);

    env->ReleaseStringUTFChars(path_part_, path_part);
    env->ReleaseStringUTFChars(merge_path_, merge_path);
}

