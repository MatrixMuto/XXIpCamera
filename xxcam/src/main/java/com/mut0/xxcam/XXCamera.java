package com.mut0.xxcam;

import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;

/**
 * Created by muto on 17-3-25.
 */

public class XXCamera {

    private static final String TAG = "XXCamera";
    private SurfaceHolder hodler;

    private CameraManager manager;
    private Surface surface;
    private CameraDevice mCameraDevice;
    private CaptureRequest.Builder prevReqBuilder;
    private HandlerThread mBackgroundThread;
    private Handler mBackgroundHandler;
    private String mCameraId;
    private long index;
    private ImageReader previewReader_;
    private XXVEncoder encoder;
    private ImageReader.OnImageAvailableListener listener;
    private CameraCaptureSession mCaptureSession;
    private int mState = STATE_PREVIEW;
    private static final int STATE_PREVIEW = 0;
    private static final int STATE_WAITING_LOCK = 1;
    private static final int STATE_WAITING_PRECAPTURE = 2;
    private static final int STATE_WAITING_NON_PRECAPTURE = 3;
    private static final int STATE_PICTURE_TAKEN = 4;
    private CaptureRequest mPreviewRequest;
    private ImageReader mJpegImageReader;

    public XXCamera(CameraManager manager, SurfaceHolder holder) {
        this.manager = manager;
        holder.addCallback(surfaceHolderCallback);
    }

    public XXCamera(CameraManager manager, SurfaceHolder holder, ImageReader.OnImageAvailableListener listener) {
        this.manager = manager;
        holder.addCallback(surfaceHolderCallback);
        this.hodler = holder;
        this.listener = listener;
    }

    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
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
    Size mJpegSize;
    public void setJpegSize(Size size) {
        mJpegSize = size;
    }

    public void open(String cameraId) {
        startBackgroundThread();
        mCameraId = cameraId;
        try {
            String[] camidlist = manager.getCameraIdList();
            String back_id = null;
            for (String id : camidlist) {
                Log.d(TAG,"camera mCameraId =[" + id +"]");
                CameraCharacteristics cc = manager.getCameraCharacteristics(id);

                StreamConfigurationMap map = cc.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                Size largestJpeg = Collections.max(
                        Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)),
                        new CompareSizesByArea());


                int mSensorOrientation = cc.get(CameraCharacteristics.SENSOR_ORIENTATION);

                Log.d(TAG, "max jpeg " + largestJpeg.getWidth() + largestJpeg.getHeight() + " " + mSensorOrientation);
//                if (facing == CameraCharacteristics.LENS_FACING_BACK) {
//                    manager.openCamera(mCameraId, cam_dev_cb, handler);
//                    break;
//                }
            }
            manager.openCamera(cameraId, cam_dev_cb, mBackgroundHandler);

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Comparator based on area of the given {@link Size} objects.
     */
    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }
    }

    public void close() {



        if (null != mCaptureSession) {
            mCaptureSession.close();
            mCaptureSession = null;
        }
        if (null != mCameraDevice) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
        if (null != mJpegImageReader) {
            mJpegImageReader.close();
            mJpegImageReader = null;
        }

        stopBackgroundThread();

    }

    private void startPreivew() {
        try {
            ArrayList<Surface> outputs = new ArrayList<>();
            //PreviewSurface
            outputs.add(hodler.getSurface());

            //preview imagereader
            previewReader_ = ImageReader.newInstance(1280, 720, ImageFormat.YUV_420_888, 10);
            previewReader_.setOnImageAvailableListener(mOnImageAvailableListener, mBackgroundHandler);
            outputs.add(previewReader_.getSurface());

//            encoder = new XXVEncoder();
//            Surface surface2 = encoder.getSurface();
//            prevReqBuilder.addTarget(surface2);

            //StillCaputre ImageReader
            mJpegImageReader = ImageReader.newInstance(mJpegSize.getWidth(), mJpegSize.getHeight(), ImageFormat.JPEG, /*maxImages*/1);
            mJpegImageReader.setOnImageAvailableListener(mOnJpegImageAvailableListener, mBackgroundHandler);
            outputs.add(mJpegImageReader.getSurface());

            mCameraDevice.createCaptureSession(outputs, createSessionCallback_, mBackgroundHandler);

//            encoder.start();

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    CameraDevice.StateCallback cam_dev_cb = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            Log.d(TAG, "onOpened");
            mCameraDevice = camera;

            if (!hodler.isCreating()) {
                startPreivew();
            }
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            Log.d(TAG, "onDisconnected");
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {

            Log.d(TAG, "onError");
        }
    };

    SurfaceHolder.Callback surfaceHolderCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.d(TAG, "surfaceCreated");
            if (mCameraDevice != null) {
                startPreivew();
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            Log.d(TAG, "surfaceChanged");
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {

        }
    };

    CameraCaptureSession.StateCallback createSessionCallback_ = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            Log.d(TAG, "onConfigured");
            mCaptureSession = session;
            try {
                prevReqBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
//                prevReqBuilder.set(CaptureRequest.JPEG_ORIENTATION, );
                prevReqBuilder.addTarget(hodler.getSurface());
//                prevReqBuilder.addTarget(previewReader_.getSurface());

                mPreviewRequest = prevReqBuilder.build();
                mCaptureSession.setRepeatingRequest(mPreviewRequest, null, mBackgroundHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {

        }
    };

    CameraCaptureSession.CaptureCallback cam_cap_cb = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureStarted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, long timestamp, long frameNumber) {
            super.onCaptureStarted(session, request, timestamp, frameNumber);
        }

        @Override
        public void onCaptureProgressed(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull CaptureResult partialResult) {
            super.onCaptureProgressed(session, request, partialResult);
        }

    };

    ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireNextImage();
            index++;
            Image.Plane[] planes = image.getPlanes();
            Log.d(TAG, "camera" + mCameraId + ":" + image.getTimestamp()
                    + "[" + image.getWidth() + "," + image.getHeight() + "]"
                    + "planes:" + planes.length
            );
            image.close();
        }
    };


    private ImageReader.OnImageAvailableListener mOnJpegImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
//            mBackgroundHandler.post(new ImageSaver(reader.acquireNextImage(), mFile));
            if (listener != null) {
                listener.onImageAvailable(reader);
            }
        }


    };

    public void takePicture() {

        captureStillPicture();
//        lockFocus();
    }

    private void lockFocus() {
        try {
            // This is how to tell the camera to lock focus.
            prevReqBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the lock.
            mState = STATE_WAITING_LOCK;
            mCaptureSession.capture(prevReqBuilder.build(), mCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void unlockFocus() {
        try {
            prevReqBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_CANCEL);
//            setAutoFlash(prevReqBuilder);
            mCaptureSession.capture(prevReqBuilder.build(), mCaptureCallback, mBackgroundHandler);
            // After this, the camera will go back to the normal state of preview.
            mState = STATE_PREVIEW;
            mCaptureSession.setRepeatingRequest(mPreviewRequest, mCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    private CameraCaptureSession.CaptureCallback mCaptureCallback
            = new CameraCaptureSession.CaptureCallback() {

        private void process(CaptureResult result) {
            switch (mState) {
                case STATE_PREVIEW: {
                    // We have nothing to do when the camera preview is working normally.
                    break;
                }
                case STATE_WAITING_LOCK: {
                    Integer afState = result.get(CaptureResult.CONTROL_AF_STATE);
                    if (afState == null) {
                        captureStillPicture();
                    } else if (CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED == afState ||
                            CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED == afState) {
                        // CONTROL_AE_STATE can be null on some devices
                        Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                        if (aeState == null ||
                                aeState == CaptureResult.CONTROL_AE_STATE_CONVERGED) {
                            mState = STATE_PICTURE_TAKEN;
                            captureStillPicture();
                        } else {
                            runPrecaptureSequence();
                        }
                    }
                    break;
                }
                case STATE_WAITING_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null ||
                            aeState == CaptureResult.CONTROL_AE_STATE_PRECAPTURE ||
                            aeState == CaptureRequest.CONTROL_AE_STATE_FLASH_REQUIRED) {
                        mState = STATE_WAITING_NON_PRECAPTURE;
                    }
                    break;
                }
                case STATE_WAITING_NON_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null || aeState != CaptureResult.CONTROL_AE_STATE_PRECAPTURE) {
                        mState = STATE_PICTURE_TAKEN;
                        captureStillPicture();
                    }
                    break;
                }
            }
        }

        @Override
        public void onCaptureProgressed(@NonNull CameraCaptureSession session,
                                        @NonNull CaptureRequest request,
                                        @NonNull CaptureResult partialResult) {
            process(partialResult);
        }

        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                       @NonNull CaptureRequest request,
                                       @NonNull TotalCaptureResult result) {
            process(result);
        }

    };

    private void captureStillPicture() {
        try {
            // This is the CaptureRequest.Builder that we use to take a picture.
            final CaptureRequest.Builder captureBuilder =
                    mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(mJpegImageReader.getSurface());

            // Use the same AE and AF modes as the preview.
            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
//            setAutoFlash(captureBuilder);

            // Orientation
//            int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
            if (mCameraId == "0") {
                captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, 90);
            }
            else {
                captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, 90);
            }

//            mCaptureSession.stopRepeating();

            mCaptureSession.capture(captureBuilder.build(), stillCaptureCallback_, null);

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    CameraCaptureSession.CaptureCallback stillCaptureCallback_ = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                       @NonNull CaptureRequest request,
                                       @NonNull TotalCaptureResult result) {
//                    showToast("Saved: " + mFile);
//                    Log.d(TAG, mFile.toString());
            unlockFocus();
        }
    };

    private void runPrecaptureSequence() {
        try {
            // This is how to tell the camera to trigger.
            prevReqBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the precapture sequence to be set.
            mState = STATE_WAITING_PRECAPTURE;
            mCaptureSession.capture(prevReqBuilder.build(), mCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
}
