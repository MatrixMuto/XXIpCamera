package com.mut0.xxcam;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by muto on 17-3-25.
 */

public class XXVEncoder {

    private static final String TAG = "XXVEncoder";
    private XXRtmpPublish rtmp;
    private Surface surface;
    MediaCodec codec;

    public XXVEncoder() {
         this(null);
    }

    public XXVEncoder(XXRtmpPublish rtmp) {
        MediaFormat format = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, 320, 240);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE, 40000);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 15);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 60);

        MediaCodecList list = new MediaCodecList(MediaCodecList.REGULAR_CODECS);

        try {
            codec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
            codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            surface = codec.createInputSurface();
            codec.setCallback(mediacodecCallback);
        } catch (IOException e) {
            e.printStackTrace();
        }

        this.rtmp = rtmp;
    }

    Surface getSurface(){
        return surface;
    }

    void start(){
        codec.start();
    }

    void stop(){
        codec.stop();
        codec.release();
    }

    MediaCodec.Callback mediacodecCallback = new MediaCodec.Callback() {
        @Override
        public void onInputBufferAvailable(@NonNull MediaCodec codec, int index) {
            Log.d(TAG, "onInputBufferAvailable "+index);
        }

        @Override
        public void onOutputBufferAvailable(@NonNull MediaCodec codec, int index, @NonNull MediaCodec.BufferInfo info) {
            ByteBuffer byteBuffer = codec.getOutputBuffer(index);
            Log.d(TAG, "onOutputBufferAvailable "+index +" " +info.presentationTimeUs +" " + info.flags + " " + byteBuffer);
            if ( 1 == (info.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME)) {

            }

            if (rtmp != null) {
                rtmp.eatVideo(byteBuffer,  info);
            }

            codec.releaseOutputBuffer(index, false);
        }

        @Override
        public void onError(@NonNull MediaCodec codec, @NonNull MediaCodec.CodecException e) {
            Log.d(TAG, "onError ");
        }

        @Override
        public void onOutputFormatChanged(@NonNull MediaCodec codec, @NonNull MediaFormat format) {
            Log.d(TAG, "onOutputFormatChanged " + format.toString());
        }
    };

    public void setRtmp(XXRtmpPublish rtmp) {
        this.rtmp = rtmp;
        start();
    }
}
