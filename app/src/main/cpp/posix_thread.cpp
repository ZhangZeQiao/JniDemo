//
// Created by joe on 2017/9/21.
//

#include <jni.h>
#include <stdio.h>
#include <unistd.h>

// TODO 这个要放最前面才有用
// TODO 解决 pthread_create “too many arguments, expected 1”
// TODO https://stackoverflow.com/questions/44463834/android-jni-pthread-create-too-many-arguments-expected-1
#ifndef _Nonnull
#define _Nonnull
// TODO NKD自带 POSIX多线程
#include <pthread.h>

#endif

#include <android/log.h>

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"xq",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"xq",FORMAT,##__VA_ARGS__);

JavaVM *javaVM;
jclass uuidutils_jcls_global;

// 动态库加载时会执行
// 兼容Android SDK 2.2之后，2.2没有这个函数
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGI("%s", "JNI_OnLoad");
    javaVM = vm;
    return JNI_VERSION_1_4;
}

void *th_fun(void *arg) {

    JNIEnv *env = NULL;
    // 通过JavaVM关联当前线程，获取当前线程的 JNIEnv
    javaVM->AttachCurrentThread(&env, NULL);
    // TODO 在子线程中不能用 FindClass
    jmethodID getUUID_mid = env->GetStaticMethodID
            (uuidutils_jcls_global, "getUUID", "()Ljava/lang/String;");

    char *no = (char *) arg;
    int i = 0;
    for (; i < 5; ++i) {
        LOGI("Thread %s,i:%d", no, i);
        jstring uuid_jstr = (jstring) env->CallStaticObjectMethod
                (uuidutils_jcls_global, getUUID_mid);
        char *uuid_cstr = (char *) env->GetStringUTFChars(uuid_jstr, NULL);
        LOGI("uuid_cstr：%s", uuid_cstr);
        if (i == 4) {
            // 退出线程
            // pthread_exit(0);
            goto end;
        }
        env->ReleaseStringUTFChars(uuid_jstr, uuid_cstr);
        sleep(1);
    }

    // TODO 解决：native thread exited without detaching 的报错
    end:
    // 用完要解除关联，不然会报错
    javaVM->DetachCurrentThread();
    // 退出线程
    pthread_exit(0);

}

// TODO 每一个线程都有一个独立的 JNIEnv
// JavaVM 代表的是Java虚拟机，JNIEnv所有的工作都是从JavaVM开始
// TODO 可以通过JavaVM获取到每个线程关联的JNIEnv
// TODO 如何获取JavaVM？
// 方法1、在JNI_OnLoad函数中获取，动态库加载时会运行
// 方法2、(*env)->GetJavaVM(env,&javaVM);【env->GetJavaVM(env,&javaVM);】

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_posix_1thread_POSIXThreadActivity_pthread
        (JNIEnv *env, jobject instance) {

    // 创建多线程
    pthread_t tid;
    pthread_create(&tid, NULL, th_fun, (void *) "NO1");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_posix_1thread_POSIXThreadActivity_init(JNIEnv *env, jobject instance) {

    // TODO 将要用到的类在主线程中初始化，不能在子线程中 FindClass，不然找不到类 ！！！
    // TODO 在主线程中去 FindClass
    // TODO 用反斜杠 com/xq/jnidemo/posix_thread/UUIDUtils
    jclass uuidutils_jcls = env->FindClass("com/xq/jnidemo/posix_thread/UUIDUtils");
    /// TODO 创建全局引用，才能在子线程中使用
    uuidutils_jcls_global = (jclass) env->NewGlobalRef(uuidutils_jcls);

    // TODO env->GetJavaVM(&javaVM); // 另外一种方式给 javaVM赋值

    LOGI("------初始化------");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_posix_1thread_POSIXThreadActivity_destroy(JNIEnv *env, jobject instance) {

    // TODO 释放全局引用
    env->DeleteGlobalRef(uuidutils_jcls_global);

    LOGI("------销毁------");
}