package com.xq.jnidemo.ffmpeg;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

import com.xq.jnidemo.R;

import java.io.File;

/**
 * @author 小侨
 * @time 2017/9/14  11:16
 * @desc ${TODD}
 */

public class FFmpegActivity extends AppCompatActivity {
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ffmpeg_activity);
    }

    public void onDecode(View view) {
        String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
        // String output = new File(Environment.getExternalStorageDirectory(), "output_1280x720_yuv420p.yuv").getAbsolutePath();
        // String output = new File(Environment.getExternalStorageDirectory(), "output_1280x800_yuv420p.yuv").getAbsolutePath();
        String output = new File(Environment.getExternalStorageDirectory(), "output_1280x800_yuv420p.avi").getAbsolutePath();

        VideoUtils.decode(input, output);
    }

    public void onPlayer(View view) {
        String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
        String output = new File(Environment.getExternalStorageDirectory(), "output_1280x800_yuv420p.yuv").getAbsolutePath();

        VideoUtils.player(input, output);
    }
}
