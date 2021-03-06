package com.mut0.xxcam;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import java.nio.ByteBuffer;

/**
 * Created by muto on 17-3-26.
 */

public class XXMicroPhone {

    AudioRecord ar;

    public XXMicroPhone(){
        int minsize = AudioRecord.getMinBufferSize(44100,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);

        ar = new AudioRecord(
                MediaRecorder.AudioSource.DEFAULT,
                44100,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT, minsize * 2);

        ar.startRecording();
    }

    public int readAudip(ByteBuffer nBuf) {
        int readed = ar.read(nBuf, 4096);

        Log.d("xxx", "readed" + readed);
        return readed;
    }

}
