package com.mut0.xxcam;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
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
import android.view.TextureView;

import org.webrtc.EglBase;
import org.webrtc.SurfaceTextureHelper;
import org.webrtc.SurfaceViewRenderer;
import org.webrtc.VideoRenderer;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Created by muto on 17-3-25.
 */

public class XXCamera implements TextureView.SurfaceTextureListener {

    private static final String TAG = "XXCamera";

    private static final boolean EANBLE_ENCODER = false;
    private static final boolean EANBLE_PREVIEW_READER = false;

    private SurfaceViewRenderer renderer;
    private XXCamTextureView textureView;
    private SurfaceHolder hodler;
    private CameraManager manager;
    private Surface surface;
    private CameraDevice mCameraDevice;
    private CaptureRequest.Builder previewReqBuilder;
    private HandlerThread mBackgroundThread;
    private Handler mBackgroundHandler;
    private String mCameraId;
    private long index;
    private ImageReader previewReader;
    private XXVEncoder encoder;
    private BokehSnapshotListener listener;
    private CameraCaptureSession mCaptureSession;
    private int mState = STATE_PREVIEW;
    private static final int STATE_PREVIEW = 0;
    private static final int STATE_WAITING_LOCK = 1;
    private static final int STATE_WAITING_PRECAPTURE = 2;
    private static final int STATE_WAITING_NON_PRECAPTURE = 3;
    private static final int STATE_PICTURE_TAKEN = 4;
    private CaptureRequest mPreviewRequest;
    private ImageReader snapshotJpegReader;
    private ImageReader snapshoYuvReader;
    private Size mJpegSize;
    private XXRtmpPublish rtmp;
    private EglBase rootEglBase;
    private static Object object = new Object();
    private static final int MAX_PREVIEW_WIDTH = 1920;
    private static final int MAX_PREVIEW_HEIGHT = 1080;
    private Size mPreviewSize;
    private boolean mFlashSupported;
    private SurfaceTextureHelper helper;


    public XXCamera(CameraManager manager, SurfaceHolder holder) {
        this.manager = manager;
        holder.addCallback(surfaceHolderCallback);
    }

    public XXCamera(CameraManager manager, SurfaceHolder holder, BokehSnapshotListener listener) {
        this.manager = manager;
        holder.addCallback(surfaceHolderCallback);
        this.hodler = holder;
        this.listener = listener;
    }

    public XXCamera(CameraManager manager, XXCamTextureView textureView) {
        this.manager = manager;
        textureView.setSurfaceTextureListener(this);
        this.textureView = textureView;
    }

    public XXCamera(CameraManager manager, SurfaceViewRenderer renderer) {
        this.manager = manager;
        this.renderer = renderer;
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

    public void setJpegSize(Size size) {
        mJpegSize = size;
    }

    private SurfaceTextureHelper.OnTextureFrameAvailableListener textureFrameAvailable
            = new SurfaceTextureHelper.OnTextureFrameAvailableListener() {
        @Override
        public void onTextureFrameAvailable(int oesTextureId, float[] transformMatrix, long timestampNs) {
            Log.d(TAG, "textureFrameAvailable");
            renderer.renderFrame(new VideoRenderer.I420Frame(640, 480, 0, oesTextureId, transformMatrix, 0));
            helper.returnTextureFrame();
        }
    };

    private Runnable openCameraRunnable = new Runnable() {
        @Override
        public void run() {
            if (renderer != null) {
                helper = SurfaceTextureHelper.create("surf" + mCameraId, rootEglBase.getEglBaseContext());
                helper.startListening(textureFrameAvailable);
                SurfaceTexture texture = helper.getSurfaceTexture();
                texture.setDefaultBufferSize(640, 480);
                surface = new Surface(texture);
            }
            synchronized (object) {
                try {
                    String[] camidlist = manager.getCameraIdList();
                    for (String id : camidlist) {
                        Log.d(TAG, "camera mCameraId =[" + id + "]");
                        CameraCharacteristics cc = manager.getCameraCharacteristics(id);
                        StreamConfigurationMap map = cc.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                        Size largestJpeg = Collections.max(Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)), new CompareSizesByArea());
                        int mSensorOrientation = cc.get(CameraCharacteristics.SENSOR_ORIENTATION);
                        Log.d(TAG, "max jpeg " + largestJpeg.getWidth() + largestJpeg.getHeight() + " " + mSensorOrientation);

                        if (id.equals(mCameraId)) {
                        }
                    }
                    manager.openCamera(mCameraId, deviceCallback, mBackgroundHandler);
                } catch (CameraAccessException | SecurityException | NullPointerException e) {
                    e.printStackTrace();
                }
            }
        }
    };

    public void open(final String cameraId) {
        startBackgroundThread();
        mCameraId = cameraId;
        if (renderer != null) {
            rootEglBase = EglBase.create();
            renderer.init(rootEglBase.getEglBaseContext(), null);
        }
        mBackgroundHandler.post(openCameraRunnable);
    }

    public void setRtmp(XXRtmpPublish rtmp) {
        this.rtmp = rtmp;
        encoder.setRtmp(rtmp);
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        this.surface = new Surface(surface);
        if (mCameraDevice != null) {
            createCaptureSession();
        }
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

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
        if (null != snapshotJpegReader) {
            snapshotJpegReader.close();
            snapshotJpegReader = null;
        }

        surface = null;
        if (null != mBackgroundThread) {
            stopBackgroundThread();
        }
    }

    private void createCaptureSession() {
        try {
            ArrayList<Surface> outputs = new ArrayList<>();

            if (textureView != null) {
                setUpCameraOutputs(textureView.getWidth(), textureView.getHeight());
                SurfaceTexture texture = textureView.getSurfaceTexture();
                texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
            }
            outputs.add(surface);

            //preview imagereader
            if (EANBLE_PREVIEW_READER) {
                previewReader = ImageReader.newInstance(1280, 720, ImageFormat.YUV_420_888, 10);
                previewReader.setOnImageAvailableListener(mOnImageAvailableListener, mBackgroundHandler);
                outputs.add(previewReader.getSurface());
            }


            if (EANBLE_ENCODER) {
                encoder = new XXVEncoder();
                outputs.add(encoder.getSurface());
            }

            //StillCaputre ImageReader
            snapshotJpegReader = ImageReader.newInstance(mJpegSize.getWidth(), mJpegSize.getHeight(), ImageFormat.JPEG, /*maxImages*/1);
            snapshotJpegReader.setOnImageAvailableListener(mOnJpegImageAvailableListener, mBackgroundHandler);
            outputs.add(snapshotJpegReader.getSurface());

//            snapshoYuvReader = ImageReader.newInstance(mJpegSize.getWidth(), mJpegSize.getHeight(), ImageFormat.RAW12, /*maxImages*/1);
//            snapshoYuvReader.setOnImageAvailableListener(mOnJpegImageAvailableListener, mBackgroundHandler);
//            outputs.add(snapshoYuvReader.getSurface());

            mCameraDevice.createCaptureSession(outputs, captureSessionCallback, mBackgroundHandler);

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Sets up member variables related to camera.
     *
     * @param width  The width of available size for camera preview
     * @param height The height of available size for camera preview
     */
    private void setUpCameraOutputs(int width, int height) {
        try {
            CameraCharacteristics characteristics
                    = manager.getCameraCharacteristics(mCameraId);

            // We don't use a front facing camera in this sample.
            Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
            if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                return;
            }

            StreamConfigurationMap map = characteristics.get(
                    CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            if (map == null) {
                return;
            }

            Size[] previewSize = map.getOutputSizes(SurfaceTexture.class);
            if (previewSize != null) {
                for (Size size : previewSize) {
                    Log.d(TAG, "preview size: " + size.getWidth() + "x" + size.getHeight());
                }
                mPreviewSize = previewSize[0];
            }


//                // For still image captures, we use the largest available size.
//                Size largest = Collections.max(
//                        Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)),
//                        new CompareSizesByArea());
//                mImageReader = ImageReader.newInstance(largest.getWidth(), largest.getHeight(),
//                        ImageFormat.JPEG, /*maxImages*/2);
//                mImageReader.setOnImageAvailableListener(
//                        mOnImageAvailableListener, mBackgroundHandler);

            // Find out if we need to swap dimension to get the preview size relative to sensor
            // coordinate.
//                int displayRotation = activity.getWindowManager().getDefaultDisplay().getRotation();
//                //noinspection ConstantConditions
//                mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
//                boolean swappedDimensions = false;
//                switch (displayRotation) {
//                    case Surface.ROTATION_0:
//                    case Surface.ROTATION_180:
//                        if (mSensorOrientation == 90 || mSensorOrientation == 270) {
//                            swappedDimensions = true;
//                        }
//                        break;
//                    case Surface.ROTATION_90:
//                    case Surface.ROTATION_270:
//                        if (mSensorOrientation == 0 || mSensorOrientation == 180) {
//                            swappedDimensions = true;
//                        }
//                        break;
//                    default:
//                        Log.e(TAG, "Display rotation is invalid: " + displayRotation);
//                }

//            Point displaySize = new Point();
//            activity.getWindowManager().getDefaultDisplay().getSize(displaySize);
//            int rotatedPreviewWidth = width;
//            int rotatedPreviewHeight = height;
//            int maxPreviewWidth = displaySize.x;
//            int maxPreviewHeight = displaySize.y;
//
//            if (swappedDimensions) {
//                rotatedPreviewWidth = height;
//                rotatedPreviewHeight = width;
//                maxPreviewWidth = displaySize.y;
//                maxPreviewHeight = displaySize.x;
//            }
//
//            if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
//                maxPreviewWidth = MAX_PREVIEW_WIDTH;
//            }
//
//            if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
//                maxPreviewHeight = MAX_PREVIEW_HEIGHT;
//            }

            // Danger, W.R.! Attempting to use too large a preview size could  exceed the camera
            // bus' bandwidth limitation, resulting in gorgeous previews but the storage of
            // garbage capture data.
//            mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
//                    rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
//                    maxPreviewHeight, largest);

            // We fit the aspect ratio of TextureView to the size of preview we picked.
//                int orientation = getResources().getConfiguration().orientation;
//                if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
//                    mTextureView.setAspectRatio(
//                            mPreviewSize.getWidth(), mPreviewSize.getHeight());
//                } else {
//                    mTextureView.setAspectRatio(
//                            mPreviewSize.getHeight(), mPreviewSize.getWidth());
//                }

            // Check if the flash is supported.
            Boolean available = characteristics.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
            mFlashSupported = available == null ? false : available;
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            // Currently an NPE is thrown when the Camera2API is used but not supported on the
            // device this code runs.
//            ErrorDialog.newInstance(getString(R.string.camera_error))
//                    .show(getChildFragmentManager(), FRAGMENT_DIALOG);
            e.printStackTrace();
        }
    }

    /**
     * Given {@code choices} of {@code Size}s supported by a camera, choose the smallest one that
     * is at least as large as the respective texture view size, and that is at most as large as the
     * respective max size, and whose aspect ratio matches with the specified value. If such size
     * doesn't exist, choose the largest one that is at most as large as the respective max size,
     * and whose aspect ratio matches with the specified value.
     *
     * @param choices           The list of sizes that the camera supports for the intended output
     *                          class
     * @param textureViewWidth  The width of the texture view relative to sensor coordinate
     * @param textureViewHeight The height of the texture view relative to sensor coordinate
     * @param maxWidth          The maximum width that can be chosen
     * @param maxHeight         The maximum height that can be chosen
     * @param aspectRatio       The aspect ratio
     * @return The optimal {@code Size}, or an arbitrary one if none were big enough
     */
    private static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, Size aspectRatio) {

        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            Log.e(TAG, "Couldn't find any suitable preview size");
            return choices[0];
        }
    }

    public void startEncoder() {

        previewReqBuilder.addTarget(encoder.getSurface());
        mPreviewRequest = previewReqBuilder.build();
        try {
            mCaptureSession.setRepeatingRequest(mPreviewRequest, previewCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    public void stopEncoder() {
        previewReqBuilder.removeTarget(encoder.getSurface());
        mPreviewRequest = previewReqBuilder.build();
        try {
            mCaptureSession.setRepeatingRequest(mPreviewRequest, previewCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    private CameraDevice.StateCallback deviceCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            Log.d(TAG, "onOpened");
            mCameraDevice = camera;

            if (surface != null) {
                createCaptureSession();
            }

        }

        @Override
        public void onClosed(@NonNull CameraDevice camera) {
            super.onClosed(camera);
            Log.d(TAG, "onClosed");

        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            Log.d(TAG, "onDisconnected");
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {

            Log.d(TAG, "onError ");
        }
    };

    private SurfaceHolder.Callback surfaceHolderCallback = new SurfaceHolder.Callback() {
        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.d(TAG, "surfaceCreated");
            surface = holder.getSurface();
            if (mCameraDevice != null) {
                createCaptureSession();
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            Log.d(TAG, "surfaceChanged");
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.d(TAG, "surfaceDestroyed");
            close();
        }
    };

    private static int configured = 0;
    private CameraCaptureSession.StateCallback captureSessionCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            Log.d(TAG, "onConfigured " + mCameraId);
            while (configured < 2) {
                synchronized (object) {
                    configured++;
                }
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            Log.d(TAG, "onConfigured " + mCameraId);
            mCaptureSession = session;
            try {
                previewReqBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                previewReqBuilder.addTarget(surface);
                if (EANBLE_PREVIEW_READER) {
                    previewReqBuilder.addTarget(previewReader.getSurface());
                }
                mPreviewRequest = previewReqBuilder.build();
                mCaptureSession.setRepeatingRequest(mPreviewRequest, null, mBackgroundHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {

        }
    };

    private ImageReader.OnImageAvailableListener mOnJpegImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
//            mBackgroundHandler.post(new ImageSaver(reader.acquireNextImage(), mFile));
            if (listener != null) {
                listener.onImageAvailable(mCameraId, reader);
            }
        }
    };


    private ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
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
    private ImageReader.OnImageAvailableListener mOnYuvImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
//            mBackgroundHandler.post(new ImageSaver(reader.acquireNextImage(), mFile));
            if (listener != null) {
                listener.onImageAvailable(mCameraId, reader);
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
            previewReqBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_START);
            // Tell #previewCaptureCallback to wait for the lock.
            mState = STATE_WAITING_LOCK;
            mCaptureSession.capture(previewReqBuilder.build(), previewCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void unlockFocus() {
        try {
            previewReqBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_CANCEL);
//            setAutoFlash(previewReqBuilder);
            mCaptureSession.capture(previewReqBuilder.build(), previewCaptureCallback, mBackgroundHandler);
            // After this, the camera will go back to the normal state of preview.
            mState = STATE_PREVIEW;
            mCaptureSession.setRepeatingRequest(mPreviewRequest, previewCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    private CameraCaptureSession.CaptureCallback previewCaptureCallback = new CameraCaptureSession.CaptureCallback() {

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
            final CaptureRequest.Builder captureBuilder =
                    mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(snapshotJpegReader.getSurface());

            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                    CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);

            // Orientation
//            int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
//            if (mCameraId == "0") {
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, 90);
//            } else {
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, 90);
//            }

            mCaptureSession.stopRepeating();

            mCaptureSession.capture(captureBuilder.build(), stillCaptureCallback_, mBackgroundHandler);

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private CameraCaptureSession.CaptureCallback stillCaptureCallback_ = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session,
                                       @NonNull CaptureRequest request,
                                       @NonNull TotalCaptureResult result) {
            unlockFocus();
        }
    };

    private void runPrecaptureSequence() {
        try {
            // This is how to tell the camera to trigger.
            previewReqBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
            // Tell #previewCaptureCallback to wait for the precapture sequence to be set.
            mState = STATE_WAITING_PRECAPTURE;
            mCaptureSession.capture(previewReqBuilder.build(), previewCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }
}
