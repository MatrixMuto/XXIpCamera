package com.mut0.xxcam2;

/**
 * Created by muto on 17-3-25.
 */

public class XXFlvMux {
    static {
        System.loadLibrary("native-lib");
    }
    public static native int nativeTest();
}
