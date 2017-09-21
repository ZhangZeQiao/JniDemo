package com.xq.jnidemo.posix_thread;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import com.xq.jnidemo.R;

/**
 * @author 小侨
 * @time 2017/9/21  10:44
 * @desc POSIX接口 JNI中多线程相关操作
 */

public class POSIXThreadActivity extends AppCompatActivity {

    static {
        System.loadLibrary("posix_thread");
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.posix_thread_activity);
        init();
    }

    public void onThread(View view) {
        pthread();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        destroy();
    }

    public native void pthread();

    public native void init();

    public native void destroy();
}
