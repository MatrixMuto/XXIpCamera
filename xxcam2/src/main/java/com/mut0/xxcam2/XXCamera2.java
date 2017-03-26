package com.mut0.xxcam2;

import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.util.Arrays;

/**
 * Created by muto on 17-3-25.
 */

public class XXCamera2 {


    private CameraManager manager;
    private Handler handler;
    private Surface surface;
    private CameraDevice camera_device;
    private CaptureRequest.Builder mPreviewRequestBuilder;


    public XXCamera2(CameraManager manager, SurfaceHolder holder) {
        this.manager = manager;
        holder.addCallback(surfaceHolderCallback);
    }

    public void init() {
        handler = new Handler();
        try {
            String[] camidlist = manager.getCameraIdList();
            String back_id = null;
            for (String id : camidlist) {
                CameraCharacteristics cc = manager.getCameraCharacteristics(id);
                Integer facing = cc.get(CameraCharacteristics.LENS_FACING);
                if (facing == CameraCharacteristics.LENS_FACING_BACK) {

                    manager.openCamera(id, cam_dev_cb, handler);
                    break;
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    CameraDevice.StateCallback cam_dev_cb = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            Log.d("xxxx", "onOpened");
            camera_device = camera;
            if (surface != null) {
                startPreivew();
            }
//            camera.createCaptureSession();
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
            surface = holder.getSurface();
            if (camera_device != null) {
                startPreivew();
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {

        }
    };

    XXVEncoder encoder;

    private void startPreivew() {
        try {
            mPreviewRequestBuilder
                    = camera_device.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewRequestBuilder.addTarget(surface);

            encoder = new XXVEncoder();
            Surface surface2 = encoder.getSurface();
            mPreviewRequestBuilder.addTarget(surface2);
            camera_device.createCaptureSession(Arrays.asList(surface,surface2), cam_cap_sess_cb, handler);
//            camera_device.createCaptureSession(Arrays.asList(surface), cam_cap_sess_cb, handler);
            encoder.start();

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

    }

    CameraCaptureSession.StateCallback cam_cap_sess_cb = new CameraCaptureSession.StateCallback() {

        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {

            try {
                session.setRepeatingRequest(
                        mPreviewRequestBuilder.build(), null, handler);

            } catch (CameraAccessException e) {
                e.printStackTrace();
            }

        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {

        }
    };
}
