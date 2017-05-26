package com.mut0.xxcam;

import android.content.Context;
import android.media.Image;
import android.media.ImageReader;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.LinkedList;

/**
 * Created by wq1950 on 17-5-9.
 */

public class BokehSnapshot implements BokehSnapshotListener {

    public final static String TAG = "BokehSnapshot";
    private static final String MAIN_CAMERA_ID = "0";
    private static final String SUB_CAMERA_ID = "2";
    private Context context;
    private LinkedList<Image> mainImages = new LinkedList<>();
    private LinkedList<Image> subImages = new LinkedList<>();
    private Object mutex = new Object();
    private HandlerThread mBackgroundThread;
    private Handler mBackgroundHandler;

    public BokehSnapshot(Context activity) {
        startBackgroundThread();
        this.context = activity;
    }

    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("BokehSnapshot");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    private void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onImageAvailable(String cameraId, ImageReader reader) {

        Image image = reader.acquireNextImage();
        Log.d(TAG, "onImageAvailable cameraId " + cameraId + " ts:" + image.getTimestamp());
        if (cameraId == MAIN_CAMERA_ID) {
            synchronized (mainImages) {
                mainImages.addLast(image);
            }
        } else if (cameraId == SUB_CAMERA_ID) {
            synchronized (subImages) {
                subImages.addLast(image);
            }
        }
        mBackgroundHandler.post(runnable);
    }

    Runnable runnable = new Runnable() {
        @Override
        public void run() {
            Image main;
            Image sub;
            if (!mainImages.isEmpty() && !subImages.isEmpty()) {
                synchronized (mainImages) {
                    main = mainImages.pop();
                }
                synchronized (subImages) {
                    sub = subImages.pop();
                }

                long mainTs = main.getTimestamp();
                long subTs = sub.getTimestamp();
                Log.d(TAG, "mainTs - subTs =" + (mainTs - subTs));
                ByteBuffer mainByteBuffer = main.getPlanes()[0].getBuffer();
                ByteBuffer subByteBuffer = sub.getPlanes()[0].getBuffer();
//                Native.bokeh_process(
//                        mainByteBuffer, mainByteBuffer.remaining(), main.getWidth(), main.getHeight(),
//                        subByteBuffer, subByteBuffer.remaining(), sub.getWidth(), sub.getHeight());
                onImageAvailable(MAIN_CAMERA_ID, main);
                onImageAvailable(SUB_CAMERA_ID, sub);
//                native_process(main.)
                main.close();
                sub.close();
            }
        }
    };

    private void native_bokeh_process(ByteBuffer mainByteBuffer, int remaining, int width, int height, ByteBuffer subByteBuffer, int remaining1, int width1, int height1) {

    }


    public void onImageAvailable(String cameraId, Image image) {
        boolean success = false;
        String filename = "JPEG_" + cameraId + "_" + System.currentTimeMillis() + ".jpg";
        File file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)
                , filename);
        ByteBuffer buffer = image.getPlanes()[0].getBuffer();
        byte[] bytes = new byte[buffer.remaining()];
        buffer.get(bytes);
        FileOutputStream output = null;
        try {
            output = new FileOutputStream(file);
            output.write(bytes);
            success = true;
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            image.close();
            closeOutput(output);
        }
        // If saving the file succeeded, update MediaStore.
        if (success) {
            MediaScannerConnection.scanFile(context, new String[]{file.getPath()},
                /*mimeTypes*/null, new MediaScannerConnection.MediaScannerConnectionClient() {
                        @Override
                        public void onMediaScannerConnected() {
                            // Do nothing
                        }

                        @Override
                        public void onScanCompleted(String path, Uri uri) {
                            Log.i(TAG, "Scanned " + path + ":");
                            Log.i(TAG, "-> uri=" + uri);
                            showToast(path);
                        }
                    });
        }
    }

    private void showToast(String text) {
        // We show a Toast by sending request message to mMessageHandler. This makes sure that the
        // Toast is shown on the UI thread.
        Message message = Message.obtain();
        message.obj = text;
        mMessageHandler.sendMessage(message);
    }


    /**
     * A {@link Handler} for showing {@link Toast}s on the UI thread.
     */
    private final Handler mMessageHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            Toast.makeText(context, (String) msg.obj, Toast.LENGTH_SHORT).show();
        }
    };

    private void closeOutput(OutputStream outputStream) {
        if (null != outputStream) {
            try {
                outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

}
