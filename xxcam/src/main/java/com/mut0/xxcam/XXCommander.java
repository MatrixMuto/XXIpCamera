package com.mut0.xxcam;

import java.nio.ByteBuffer;

/**
 * Created by wq1950 on 17-4-18.
 */

public class XXCommander {

    public XXCommander() {
        XXMicroPhone mp = new XXMicroPhone();

        XXAEncoder ae = new XXAEncoder();

//        XXCamera camera = new XXCamera();
        XXVEncoder ve = new XXVEncoder();



        XXRtmpPublish rtmp = new XXRtmpPublish();

    }


    void data(ByteBuffer buf){

    }
}
