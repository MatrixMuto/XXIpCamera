package com.example.muto.xxipcamera;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.util.Size;
import android.view.Surface;

import com.mut0.xxcam.XXCamTextureView;
import com.mut0.xxcam.XXCamera;

import junit.framework.Assert;

import org.webrtc.EglBase;
import org.webrtc.SurfaceTextureHelper;
import org.webrtc.SurfaceViewRenderer;
import org.webrtc.VideoRenderer;

public class EglActivity extends AppCompatActivity {

    private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;
    private static final String TAG = "EglActivity";

    XXCamera camera0;
    XXCamera camera2;
    private SurfaceViewRenderer preview0;
    private SurfaceViewRenderer preview2;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_egl);
        preview0 = (SurfaceViewRenderer) findViewById(R.id.preview0);
        preview2 = (SurfaceViewRenderer) findViewById(R.id.preview2);
    }

    @Override
    protected void onResume() {
        super.onResume();
        startCamera();
    }

    @Override
    protected void onPause() {
        super.onPause();
        stopCamera();
    }

    private void startCamera() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED
                || ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    this,
                    new String[]{Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    PERMISSION_REQUEST_CODE_CAMERA);
            return;
        }
        openCamera0("0");
//        openCamera2("1");
    }

    private void openCamera0(String id){
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        camera0 = new XXCamera(manager, preview0);
        camera0.setJpegSize(new Size(4160, 3120));
        camera0.open("1");
    }

    private void openCamera2(String id){
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        camera2 = new XXCamera(manager, preview2);
        camera2.setJpegSize(new Size(1600, 1200));
        camera2.open("2");
    }

    private void stopCamera() {
//        camera.close();
//        surface.release();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if (PERMISSION_REQUEST_CODE_CAMERA != requestCode) {
            super.onRequestPermissionsResult(requestCode,
                    permissions,
                    grantResults);
            return;
        }
        Assert.assertEquals(grantResults.length, 2);
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            startCamera();
        }
    }
}
