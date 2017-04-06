package com.mut0.xxcam2;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.support.annotation.NonNull;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by muto on 17-3-26.
 */

public class XXAEncoder {

    MediaCodec codec;
    XXMicroPhone pp;

    public XXAEncoder(){
        MediaFormat format = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, 48000, 2);
        format.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
        format.setInteger(MediaFormat.KEY_BIT_RATE, 48000);
        MediaCodecList list = new MediaCodecList(MediaCodecList.REGULAR_CODECS);

//        String encoder_type = list.findEncoderForFormat(format);

        try {
            codec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);

            codec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

            codec.setCallback(mediacodecCallback);

            pp = new XXMicroPhone();

            codec.start();
        } catch (IOException e) {
            e.printStackTrace();
        }


    }


    MediaCodec.Callback mediacodecCallback = new MediaCodec.Callback() {
        @Override
        public void onInputBufferAvailable(@NonNull MediaCodec codec, int index) {
            Log.d("xxxx", "onInputBufferAvailable "+index);

            ByteBuffer buffer = codec.getInputBuffer(index);

            int size = pp.readAudip(buffer);

            codec.queueInputBuffer(index, buffer.position(), size, 0, 0);

        }

        @Override
        public void onOutputBufferAvailable(@NonNull MediaCodec codec, int index, @NonNull MediaCodec.BufferInfo info) {
//                info.presentationTimeUs
            Log.d("xxxxA", "onOutputBufferAvailable "+index +" " +info.presentationTimeUs +" " + info.flags);
            ByteBuffer buffer = codec.getOutputBuffer(index);

            codec.releaseOutputBuffer(index, false);
        }

        @Override
        public void onError(@NonNull MediaCodec codec, @NonNull MediaCodec.CodecException e) {
            Log.d("xxxx", "onError ");
        }

        @Override
        public void onOutputFormatChanged(@NonNull MediaCodec codec, @NonNull MediaFormat format) {
            Log.d("xxxx", "onOutputFormatChanged " + format.toString());
        }
    };
}
