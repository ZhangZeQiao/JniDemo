package com.xq.jnidemo.file_encrypt;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Toast;

import com.xq.jnidemo.R;

import java.io.File;

/**
 * @author 小侨
 * @time 2017/9/4  17:27
 * @desc 文件加密解密
 */

public class FileEncryptActivity extends AppCompatActivity {

    static {
        System.loadLibrary("file-encrypt");
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_file_encrypt);
    }

    public void onEncrypt(View view) {
        String normal_path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "zzz.png";
        String crypt_path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "zzz_encrypt.png";
        encrypt(normal_path, crypt_path);
        Toast.makeText(this, crypt_path, Toast.LENGTH_LONG).show();
    }

    public void onDecrypt(View view) {
        String crypt_path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "zzz_encrypt.png";
        String decrypt_path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separatorChar + "zzz_decrypt.png";
        decrypt(crypt_path, decrypt_path);
        Toast.makeText(this, decrypt_path, Toast.LENGTH_LONG).show();
    }

    public native static void encrypt(String normal_path, String crypt_path);

    public native static void decrypt(String crypt_path, String decrypt_path);

}
