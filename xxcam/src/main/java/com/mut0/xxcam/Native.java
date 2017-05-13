package com.mut0.xxcam;

import java.nio.ByteBuffer;

/**
 * Created by wq1950 on 17-5-9.
 */

public class Native {
    public static native void bokeh_process(
            ByteBuffer mainByteBuffer, int remaining, int width, int height,
            ByteBuffer subByteBuffer, int remaining1, int width1, int height1);
}
