package com.xq.jnidemo;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

public class JniActivity extends AppCompatActivity {

    // 加载动态库
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        initIds();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        doTest();
    }

    // TODO ：test
    private void doTest() {
        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(myStringFromJNI());

        int[] array = {9, 100, 10, 37, 5, 10};
        giveArray(array);
        for (int i : array) {
            Log.v("zzq : ", i + "");
        }

        int[] array2 = getArray(10);
        for (int i : array2) {
            Log.v("zzq : ", i + "");
        }

        try {
            exception();
        } catch (Exception e) {
            Log.v("zzq", e.getMessage());
        }


        // 不断调用cached方法
        for (int i = 0; i < 100; i++) {
            cached();
        }

        Log.v("zzq", "---");

        includeH();
    }

    // TODO ：java调用c/c++

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI(int num);

    public native String myStringFromJNI();

    public native static String staticStringFromJNI();

    // TODO ：c/c++访问java的成员
    public String mName = "ZhangZeQiao";

    public String sName = "ZhangZeQiao";

    public native int accessFieldFromJNI();

    public native static int accessStaticFieldFromJNI();

    // TODO ：访问java方法
    public native void showToastFromJNI();

    public void showToast(String meg) {
        Toast.makeText(this, meg, Toast.LENGTH_LONG).show();
    }

    public void showToast(long time) {
        Toast.makeText(this, time + "", Toast.LENGTH_LONG).show();
    }

    // TODO ：访问java构造方法
    public native void accessConstructor();

    // TODO ：访问父类方法
    public native void accessNonvirtualMethod();

    // TODO ：中文乱码问题
    public native String chineseChars(String text);

    // TODO ：数组处理
    public native void giveArray(int[] array);

    public native int[] getArray(int len);

    // TODO ：引用变量
    public native void localRef();

    public native void createGlobalRef();

    public native String getGlobalRef();

    public native void deleteGlobalRef();

    // TODO ：异常处理
    public native void exception();

    // TODO ：缓存策略
    public native void cached();
    public native static void initIds();

    // include .h 文件
    public native void includeH();
}
