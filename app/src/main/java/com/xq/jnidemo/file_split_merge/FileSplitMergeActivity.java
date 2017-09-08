package com.xq.jnidemo.file_split_merge;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.xq.jnidemo.R;

import java.io.File;

/**
 * @author 小侨
 * @time 2017/9/5  10:15
 * @desc 文件的拆分合并
 */

public class FileSplitMergeActivity extends AppCompatActivity {

    // 加载动态库
    static {
        System.loadLibrary("file-split-merge");
    }

    private String SD_CARD_PATH;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_file_split_merge);

        SD_CARD_PATH = Environment.getExternalStorageDirectory().getAbsolutePath();
    }

    public void onSplit(View view) {
        String path = SD_CARD_PATH + File.separatorChar + "zzz.png";
        String path_part = SD_CARD_PATH + File.separatorChar + "zzz_%d.png";
        split(path, path_part, 5);
        Log.v("zzq", "拆分完毕...");
    }

    public void onMerge(View view) {
        String path_part = SD_CARD_PATH + File.separatorChar + "zzz_%d.png";
        String last_path = SD_CARD_PATH + File.separatorChar + "zzz_merge.png";
        merge(path_part, 5, last_path);
        Log.v("zzq", "合并完毕...");
    }

    /**
     * 拆分文件
     *
     * @param path      待拆分文件路径
     * @param path_part 拆分后子文件路径格式
     * @param count     拆分子文件数量
     */
    public native void split(String path, String path_part, int count);

    /**
     * 合并文件
     *
     * @param path_part 待合并的文件
     * @param count     待合并文件数
     * @param merge_path 最终合并文件存放路径
     */
    public native void merge(String path_part, int count, String merge_path);
}
