package com.mut0.xxcam;

import android.content.Context;
import android.media.ImageReader;

/**
 * Created by wq1950 on 17-5-9.
 */

public class BokehPreview implements BokehSnapshotListener {


    private final static String TAG = "BokehPreview";
    private final Context context;

    public BokehPreview(Context applicationContext) {
        this.context = applicationContext;
    }

    @Override
    public void onImageAvailable(String cameraId, ImageReader image) {

    }


}
