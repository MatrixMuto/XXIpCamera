package com.mut0.xxcam;

import android.media.Image;
import android.media.ImageReader;

/**
 * Created by wq1950 on 17-5-9.
 */

public interface BokehSnapshotListener {
    void onImageAvailable(String cameraId, ImageReader image);
}
