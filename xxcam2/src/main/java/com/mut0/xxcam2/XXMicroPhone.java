package com.mut0.xxcam2;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

/**
 * Created by muto on 17-3-26.
 */

public class XXMicroPhone {

    XXMicroPhone(){
//        AudioRecord ar = new AudioRecord.Builder()
//                .setAudioSource(MediaRecorder.AudioSource.VOICE_COMMUNICATION)
//                .setAudioFormat(new AudioFormat.Builder()
//                    .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
//                    .setSampleRate(32000)
//                    .setChannelMask(AudioFormat.CHANNEL_IN_MONO)
//                    .build())
//                .build();

        AudioRecord ar = new AudioRecord(MediaRecorder.AudioSource.VOICE_COMMUNICATION
                                            ,44100,
                                            );

    }
}
