package com.mut0.xxcam;

import android.media.MediaCodec;

import java.nio.ByteBuffer;

/**
 * Created by muto on 17-3-25.
 */

public class XXRtmpPublish {

    static {
        System.loadLibrary("native-lib");
    }

    public XXRtmpPublish() {

    }

    public void connect(String url) {
        native_connect(url);
    }

    void disconnect() {
        native_disconnect();
    }

    public void eatVideo(ByteBuffer byteBuffer, MediaCodec.BufferInfo info) {
        native_eatVideo(byteBuffer, byteBuffer.position(), byteBuffer.remaining(), info.offset, info.flags, info.presentationTimeUs);
    }


    private native void native_connect(String url);

    private native void native_disconnect();

    private native void native_eatVideo(ByteBuffer byteBuffer, int position, int remaining, int offset, int flags, long presentationTimeUs);

    public void eatAudio(ByteBuffer byteBuffer, MediaCodec.BufferInfo info) {
        native_eatAudio(byteBuffer, byteBuffer.position(), byteBuffer.remaining(), info.offset, info.flags, info.presentationTimeUs);
    }

    private native void native_eatAudio(ByteBuffer byteBuffer, int position, int remaining, int offset, int flags, long presentationTimeUs);
}
