package com.mut0.xxcam;

import android.media.MediaCodec;

import java.nio.ByteBuffer;

/**
 * Created by muto on 17-3-25.
 */

public class XXFlvMux {
    static {
        System.loadLibrary("native-lib");
    }
    public static native int nativeTest();

    public void eat(ByteBuffer byteBuffer) {

    }

    public void eatVideo(ByteBuffer byteBuffer, MediaCodec.BufferInfo info) {
        nativeEatVideo(byteBuffer);
    }

    private native void nativeEatVideo(ByteBuffer byteBuffer);
}
