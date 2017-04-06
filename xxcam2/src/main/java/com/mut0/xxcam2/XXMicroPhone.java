package com.mut0.xxcam2;

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
        int minsize = AudioRecord.getMinBufferSize(48000,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);

        ar = new AudioRecord(
                MediaRecorder.AudioSource.DEFAULT,
                48000,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,minsize*2);
        ar.startRecording();
//        XXXThread t  = new XXXThread();
//        t.start();
    }

    public int readAudip(ByteBuffer nBuf) {
        int readed = ar.read(nBuf, 4096);

        Log.d("xxx", "readed" + readed);
        return readed;
    }


    private static class XXXThread extends Thread {
        @Override
        public void run() {
            super.run();
        }
    }

}
