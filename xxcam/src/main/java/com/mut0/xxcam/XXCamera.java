package com.mut0.xxcam;

import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.widget.Toast;

import java.nio.ByteBuffer;
import java.util.ArrayList;

/**
 * Created by muto on 17-3-25.
 */

public class XXCamera {

    private static final String TAG = "XXCamera";

    private CameraManager manager;
    private Surface surface;
    private CameraDevice camerDevice_;
    private CaptureRequest.Builder prevReqBuilder;
    private HandlerThread mBackgroundThread;
    private Handler mBackgroundHandler;
    private String id;
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

    public void init(String cam_id) {
        startBackgroundThread();
        id = cam_id;
        try {
//            String[] camidlist = manager.getCameraIdList();
//            String back_id = null;
//            for (String id : camidlist) {
//                CameraCharacteristics cc = manager.getCameraCharacteristics(id);
//                Integer facing = cc.get(CameraCharacteristics.LENS_FACING);
//                if (facing == CameraCharacteristics.LENS_FACING_BACK) {
//                    Log.d("xxxx","camera id =[" + id +"]");
//                    manager.openCamera(id, cam_dev_cb, handler);
//                    break;
//                }
//
//            }
            manager.openCamera(cam_id, cam_dev_cb, mBackgroundHandler);

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    public void close() {
        stopBackgroundThread();
    }

    private void startPreivew() {
        try {
            ArrayList<Surface> outputs = new ArrayList<>();
            //PreviewSurface
            outputs.add(surface);

            //preview imagereader
            previewReader_ = ImageReader.newInstance(1280, 720, ImageFormat.YUV_420_888, 10);
            previewReader_.setOnImageAvailableListener(mOnImageAvailableListener, mBackgroundHandler);
            outputs.add(previewReader_.getSurface());

//            encoder = new XXVEncoder();
//            Surface surface2 = encoder.getSurface();
//            prevReqBuilder.addTarget(surface2);

            //StillCaputre ImageReader
            mJpegImageReader = ImageReader.newInstance(640, 480, ImageFormat.JPEG, /*maxImages*/1);
            mJpegImageReader.setOnImageAvailableListener(mOnJpegImageAvailableListener, mBackgroundHandler);
            outputs.add(mJpegImageReader.getSurface());

            camerDevice_.createCaptureSession(outputs, createSessionCallback_, mBackgroundHandler);

//            encoder.start();

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    CameraDevice.StateCallback cam_dev_cb = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            Log.d("xxxx", "onOpened");
            camerDevice_ = camera;
            if (surface != null) {
                startPreivew();
            }
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            Log.d("xxxx", "onDisconnected");
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {

            Log.d("xxxx", "onError");
        }
    };

    SurfaceHolder.Callback surfaceHolderCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.d("xxxx", "surfaceCreated");
            surface = holder.getSurface();
            if (camerDevice_ != null) {
                startPreivew();
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            Log.d("xxxx", "surfaceChanged");
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {

        }
    };

    CameraCaptureSession.StateCallback createSessionCallback_ = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            mCaptureSession = session;
            try {
                prevReqBuilder = camerDevice_.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                prevReqBuilder.addTarget(surface);
                prevReqBuilder.addTarget(previewReader_.getSurface());

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
            Log.d("xxxx", "camera" + id + ":" + image.getTimestamp()
                    + "[" + image.getWidth() + "," + image.getHeight() + "]"
                    + "planes:" + planes.length
            );
            image.close();
        }
    };


    private ImageReader.OnImageAvailableListener mOnJpegImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
//            mBackgroundHandler.post(new ImageSaver(previewReader_.acquireNextImage(), mFile));
            Image image = reader.acquireNextImage();

            ByteBuffer buffer = image.getPlanes()[0].getBuffer();
            byte[] bytes = new byte[buffer.remaining()];
            buffer.get(bytes);

            image.close();
        }
    };

    public void takePicture() {
        lockFocus();
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
            // Reset the auto-focus trigger
            prevReqBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER,
                    CameraMetadata.CONTROL_AF_TRIGGER_CANCEL);
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

    private void showToast(final String text) {
//        final Activity activity = getActivity();
//        if (activity != null) {
//            activity.runOnUiThread(new Runnable() {
//                @Override
//                public void run() {
//                    Toast.makeText(activity, text, Toast.LENGTH_SHORT).show();
//                }
//            });
//        }
    }

    private void captureStillPicture() {
        try {
//            final Activity activity = getActivity();
//            if (null == activity || null == mCameraDevice) {
//                return;
//            }
            // This is the CaptureRequest.Builder that we use to take a picture.
            final CaptureRequest.Builder captureBuilder =
                    camerDevice_.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(mJpegImageReader.getSurface());

            // Use the same AE and AF modes as the preview.
            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
//            setAutoFlash(captureBuilder);

            // Orientation
//            int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, 270);

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