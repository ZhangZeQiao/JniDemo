package com.xq.jnidemo.posix_thread;

import java.util.UUID;

/**
 * @author 小侨
 * @time 2017/9/21  11:32
 * @desc ${TODD}
 */

public class UUIDUtils {

    /**
     * 在CC++生成一个UUID特别麻烦，还不如通过 jni调用 java生成
     * @return
     */
    public static String getUUID(){
        return UUID.randomUUID().toString();
    }
}
