#include <jni.h>
#include <string>
#include <stdlib.h>
#include <android/log.h> // TODO ：jni中使用 log

// JNIEnv在 C++中是结构体别名（env为一级指针），在 C中是结构体指针别名（env为二级指针）
// C++只是对 C的那一套进行的封装，给一个变量赋值为指针，这个变量是二级指针
// jni为了能够适配 C和 C++，C++又是对 C封装，又通过指针才能操作内存，C++一个指针，C就得两个

// 为什么需要传入JNIEnv？函数执行过程中需要 JNIEnv；C++为什么没有传入？有 this
// this就是为了调用 C：return functions->DefineClass(this, name, loader, buf, bufLen);
// TODO ：java调用c/c++
extern "C"
JNIEXPORT jstring JNICALL Java_com_xq_jnidemo_JniActivity_stringFromJNI
        (JNIEnv *env, jobject /* this */, jint num) {

    std::string hello = "看到我了吗";
    // 将 C的字符串转为一个 Java字符串
    if (num == 666) {
        return env->NewStringUTF("666666");
    } else {
        return env->NewStringUTF(hello.c_str());
    }
    return env->NewStringUTF(hello.c_str());
    // 相当于：return env->NewStringUTF("看到我了吗");
    // 这是 C++的实现方式，C的实现方式： return (*env)->NewStringUTF(env,"看到我了吗");
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xq_jnidemo_JniActivity_myStringFromJNI(JNIEnv *env, jobject instance) {
    std::string hello = "看到了";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_xq_jnidemo_JniActivity_staticStringFromJNI(JNIEnv *env, jclass type) {

    // 每个native函数都至少有两个参数（JNIEnv*，jclass或者jobject）
    // 1、当native方法为静态方法时：jclass代表native方法所属类的class对象（JniActivity.class）
    // 2、当native方法为非静态方法时：jobject代表native方法所属的对象/实例

    return env->NewStringUTF("看到我了吗");
}

// TODO ：c/c++访问java的成员
extern "C"
JNIEXPORT jint JNICALL
Java_com_xq_jnidemo_JniActivity_accessFieldFromJNI(JNIEnv *env, jobject instance) {

    // 得到 jclass
    jclass cls = env->GetObjectClass(instance);
    // 类名，属性名称，属性签名
    jfieldID fidID = env->GetFieldID(cls, "mName", "Ljava/lang/String;");
    jstring fid = (jstring) (env->GetObjectField(instance, fidID)); // 拿到对象/实例的属性，而不是类的属性

    const char *c_str = env->GetStringUTFChars(fid, JNI_FALSE);
    char *text = "super ";
    // 拼接字符串
    // strcat(text, c_str); // 一拼接就报错 ！！！
    // c字符串 -> jstring
    jstring new_jstring = env->NewStringUTF(text);
    // 修改mName
    env->SetObjectField(instance, fidID, new_jstring);

    // return env->NewStringUTF("修改成功");
    // return new_jstring;
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_xq_jnidemo_JniActivity_accessStaticFieldFromJNI(JNIEnv *env, jclass type) {

    // 类名，属性名称，属性签名
    jfieldID fidID = env->GetFieldID(type, "sName", "Ljava/lang/String;");
    jstring fid = (jstring) (env->GetStaticObjectField(type, fidID));

    env->SetStaticObjectField(type, fidID, fid);

    return 1;
}

// TODO ：访问java方法
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_showToastFromJNI(JNIEnv *env, jobject instance) {

    // 得到 jclass
    jclass cls = env->GetObjectClass(instance);
    // 得到方法id
    jmethodID mid = env->GetMethodID(cls, "showToast", "(Ljava/lang/String;)V");
    // 调用
    env->CallVoidMethod(instance, mid, env->NewStringUTF("调用成功"));
}

// TODO ：访问java构造方法
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_accessConstructor(JNIEnv *env, jobject instance) {

    // 使用java.util.Data产生一个当前的时间戳
    jclass cls = env->FindClass("java/util/Date");
    // 获取构造方法id
    jmethodID constructor_mid = env->GetMethodID(cls, "<init>", "()V"); // 构造方法名固定用 init
    // 实例化一个对象
    jobject date_obj = env->NewObject(cls, constructor_mid);
    // 调用getTime方法
    jmethodID gettime_mid = env->GetMethodID(cls, "getTime", "()J");
    jlong time = env->CallLongMethod(date_obj, gettime_mid);

    // printf("hahatime : %ld\n",time);
    // 得到 jclass
    jclass instance_cls = env->GetObjectClass(instance);
    // 得到方法id
    jmethodID showToast_mid = env->GetMethodID(instance_cls, "showToast", "(J)V");
    // 调用
    env->CallVoidMethod(instance, showToast_mid, time);
}

// TODO ：访问父类方法extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_accessNonvirtualMethod(JNIEnv *env, jobject instance) {

    // env->CallObjectMethod(obj,mid); // 调用方法，会自动覆盖父类方法
    // env->CallNonvirtualObjectMethod(obj,clazz,mid); // 不覆盖，直接调用父类方法
}

// TODO ：中文乱码问题
extern "C"
JNIEXPORT jstring JNICALL
Java_com_xq_jnidemo_JniActivity_chineseChars(JNIEnv *env, jobject instance, jstring text) {

    const char *c_str = env->GetStringUTFChars(text, JNI_FALSE);

    // 得到 jclass
    jclass cls = env->GetObjectClass(instance);
    // 得到方法id
    jmethodID mid = env->GetMethodID(cls, "showToast", "(Ljava/lang/String;)V");
    // 调用
    env->CallVoidMethod(instance, mid, env->NewStringUTF(c_str));

    // char* c_string = "你麻痹";
    char c_string[] = "你麻痹";
    jstring jstr = env->NewStringUTF(c_string);

    // 如果出现转换的乱码问题，可调用java的转换函数转换后，才return过去

    return jstr;
}

// TODO ：数组处理
//  比较函数，由程序猿编写，用以判断两个元素哪个更应该排在前面
int compare(const void *a, const void *b) {
    int *pa = (int *) a;
    int *pb = (int *) b;
    return (*pa) - (*pb);  // 从小到大排序
}

extern "C" // 这里要紧跟下面的代码
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_giveArray(JNIEnv *env, jobject instance, jintArray array_) {
    jint *array = env->GetIntArrayElements(array_, NULL); // 不复制，（表面上是）操作同一块内存，但操作后要同步

    // jintArray -> jint指针 -> c int 数组
    // 数组的长度
    jsize len = env->GetArrayLength(array_);
    // 排序
    qsort(array, len, sizeof(jint), compare);

    // 同步，虽是操作同一内存，但是要同步
    // 释放操作
    // 一个 Get对应一个 Release
    env->ReleaseIntArrayElements(array_, array, 0);
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_xq_jnidemo_JniActivity_getArray(JNIEnv *env, jobject instance, jint len) {

    // jni类型
    jintArray array = env->NewIntArray(len);
    // c类型
    jint *elems = env->GetIntArrayElements(array, NULL);
    int i = 0;
    for (; i < len; ++i) {
        elems[i] = i;
    }
    // jni类型和c类型的同步
    env->ReleaseIntArrayElements(array, elems, 0);
    return array;
}

// TODO ：引用变量
// 引用类型：局部引用、全局引用
// 作用：在JNI中告知虚拟机何时回收一个JNI变量
// 局部引用：通过 DeleteLocalRef手动释放
// 模拟：循环创建数组
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_localRef(JNIEnv *env, jobject instance) {

    int i = 0;
    for (; i < 5; ++i) {
        // 创建Date对象
        jclass cls = env->FindClass("java/util/Date");
        jmethodID constructor_mid = env->GetMethodID(cls, "<init>", "()V"); // 构造方法名固定用 init
        jobject date_obj = env->NewObject(cls, constructor_mid);
        // 此处省略一万行代码 ...
        // 不再使用jobject对象了
        // 通知垃圾回收器回收这些对象
        env->DeleteLocalRef(date_obj);
        // 此处省略一万行代码 ...
    }
}
// 全局引用：
jstring global_str;
// 创建
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_createGlobalRef(JNIEnv *env, jobject instance) {

    jstring str = env->NewStringUTF("jni development is powerful !");
    global_str = (jstring) env->NewGlobalRef(str);
}
// 获得
extern "C"
JNIEXPORT jstring JNICALL
Java_com_xq_jnidemo_JniActivity_getGlobalRef(JNIEnv *env, jobject instance) {

    return global_str; // 直接返回的是 jni的 jstring类型，而不是 c类型，所以不用 NewStringUTF(c_string)
}
// 释放
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_deleteGlobalRef(JNIEnv *env, jobject instance) {

    env->DeleteGlobalRef(global_str);
}
// 弱全局引用：
// 节省内存，在内存不足时释放所引用的对象
// 可以引用一个不常用的对象
// 创建：NewWeakGlobalRef  销毁：DeleteGlobalWeakRef

// TODO ：异常处理
// jni自己抛出的异常，在java层无法被捕捉到，只能在c层清空（改成 Throwable异常可以）
// 用户通过ThrowNew抛出的异常，可以在java层捕捉
// 1、保证java代码可以运行
// 2、补救措施保证c代码继续运行
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_exception(JNIEnv *env, jobject instance) {

    jclass cls = env->GetObjectClass(instance);
    jfieldID fid = env->GetFieldID(cls, "mName2", "Ljava/lang/String;");
    // 检测是否发生 java异常
    jthrowable exception = env->ExceptionOccurred();
    if (exception != NULL) {
        // 让java代码可以继续运行
        // 清空异常信息
        env->ExceptionClear();

        // 补救措施
        fid = env->GetFieldID(cls, "mName", "Ljava/lang/String;");
    }

    jstring jstr = (jstring) env->GetObjectField(instance, fid);
    const char *str = env->GetStringUTFChars(jstr, JNI_FALSE);

    // 对比属性值是否合法
    if (strcmp(str, "ZhZeQiao")) {
        // 人为抛出异常，给java层处理
        jclass newExcCls = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(newExcCls, "name错误！");
    }

}

// TODO ：缓存策略
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_cached(JNIEnv *env, jobject instance) {

    jclass cls = env->GetObjectClass(instance);
    // jfieldID fid = env->GetFieldID(cls, "mName", "Ljava/lang/String;");
    // 获取 fid只获取一次
    // static 局部的静态变量
    static jfieldID fid = NULL; // 即使函数运行完毕，还存在内存，一直到程序退出
    // 如果用 jfieldID fid = NULL;多次调用多次生成
    if (fid == NULL) {
        fid = env->GetFieldID(cls, "mName", "Ljava/lang/String;");
    }
}
// 初始化全局变量，动态库加载完成之后，立刻缓存起来
jfieldID name_fid;
jmethodID showToast_mid;
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_initIds(JNIEnv *env, jclass type) {

    name_fid = env->GetFieldID(type, "mName", "Ljava/lang/String;");
    showToast_mid = env->GetMethodID(type, "showToast", "(Ljava/lang/String;)V");
}

// TODO ：include .h 文件
#include "zzq.h"

// jni中使用 log，宏定义函数
#define LOGV(FORMAT, ...) __android_log_print(ANDROID_LOG_VERBOSE, "zzq", FORMAT, __VA_ARGS__);

void MyPrintf::myprintf() {
    LOGV("老铁 : %s", "双击666");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_JniActivity_includeH(JNIEnv *env, jobject instance) {

    MyPrintf mp;
    mp.myprintf();
}
