//
// Created by joe on 2017/9/8.
//

#include <jni.h>
#include <string>
#include <stdlib.h>
#include <android/log.h> // TODO ：jni中使用 log

// TODO ：文件加密
char *password = "iloveyou";
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_file_1encrypt_FileEncryptActivity_encrypt
        (JNIEnv *env, jclass type, jstring normal_path_, jstring crypt_path_) {
    const char *normal_path = env->GetStringUTFChars(normal_path_, 0);
    const char *crypt_path = env->GetStringUTFChars(crypt_path_, 0);

    //打开文件
    FILE *normal_fp = fopen(normal_path, "rb");
    FILE *crypt_fp = fopen(crypt_path, "wb");
    //一次读取一个字符
    int ch;
    int i = 0; //循环使用密码中的字母进行异或运算
    int pwd_len = strlen(password); //密码的长度
    while ((ch = fgetc(normal_fp)) != EOF) { //End of File
        //写入（异或运算）
        fputc(ch ^ password[i % pwd_len], crypt_fp);
        i++;
    }
    //关闭
    fclose(crypt_fp);
    fclose(normal_fp);

    env->ReleaseStringUTFChars(normal_path_, normal_path);
    env->ReleaseStringUTFChars(crypt_path_, crypt_path);
}
// TODO ：文件解密
extern "C"
JNIEXPORT void JNICALL
Java_com_xq_jnidemo_file_1encrypt_FileEncryptActivity_decrypt
        (JNIEnv *env, jclass type, jstring crypt_path_, jstring decrypt_path_) {
    const char *crypt_path = env->GetStringUTFChars(crypt_path_, 0); // 将jni类型的路径改成c类型的路径
    const char *decrypt_path = env->GetStringUTFChars(decrypt_path_, 0);

    //打开文件
    FILE *normal_fp = fopen(crypt_path, "rb");
    FILE *crypt_fp = fopen(decrypt_path, "wb");
    //一次读取一个字符
    int ch;
    int i = 0; //循环使用密码中的字母进行异或运算
    int pwd_len = strlen(password); //密码的长度
    while ((ch = fgetc(normal_fp)) != EOF) { //End of File
        //写入（异或运算）
        fputc(ch ^ password[i % pwd_len], crypt_fp);
        i++;
    }
    //关闭
    fclose(crypt_fp);
    fclose(normal_fp);

    env->ReleaseStringUTFChars(crypt_path_, crypt_path);
    env->ReleaseStringUTFChars(decrypt_path_, decrypt_path);
}