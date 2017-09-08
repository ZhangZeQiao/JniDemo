package com.xq.jnidemo;

import android.app.Application;

import com.squareup.leakcanary.LeakCanary;

/**
 * @author 小侨
 * @time 2017/9/8  16:21
 * @desc ${TODD}
 */

public class JNIDemoApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        if (LeakCanary.isInAnalyzerProcess(this)) {
            // This process is dedicated to LeakCanary for heap analysis.
            // You should not init your app in this process.
            return;
        }
        LeakCanary.install(this);
        // Normal app init code...
    }
}
