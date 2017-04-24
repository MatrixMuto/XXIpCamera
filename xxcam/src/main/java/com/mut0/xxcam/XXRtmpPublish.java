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

    public XXRtmpPublish(){

    }


    void addTarget(String url){
        native_addTarget(url);
    }


    public void connect(){
        native_connect();
    }

    void disconnect(){
        native_disconnect();
    }
    public void eatVideo(ByteBuffer byteBuffer, MediaCodec.BufferInfo info) {
        native_eatVideo(byteBuffer);
    }
    private native void native_addTarget(String url);
    private native void native_connect();
    private native void native_disconnect();
    private native void native_eatVideo(ByteBuffer byteBuffer);


}
