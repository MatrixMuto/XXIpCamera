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

    XXCamTextureView textureView;
    XXCamera camera;

    Surface surface;
    private SurfaceViewRenderer localPreview;
    EglBase rootEglBase;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_egl);
        localPreview = (SurfaceViewRenderer) findViewById(R.id.local_preview);
        rootEglBase = EglBase.create();

        localPreview.init(rootEglBase.getEglBaseContext(), null);
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

        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        final SurfaceTextureHelper helper = SurfaceTextureHelper.create("surf", rootEglBase.getEglBaseContext());
        helper.startListening(new SurfaceTextureHelper.OnTextureFrameAvailableListener() {
            @Override
            public void onTextureFrameAvailable(int oesTextureId, float[] transformMatrix, long timestampNs) {
                Log.d(TAG, "onTextureFrameAvailable");
                localPreview.renderFrame(new VideoRenderer.I420Frame(640, 480, 0, oesTextureId, transformMatrix, 0));
                helper.returnTextureFrame();
            }
        });
        SurfaceTexture texture = helper.getSurfaceTexture();
        texture.setDefaultBufferSize(640, 480);
        surface = new Surface(texture);
        camera = new XXCamera(manager, surface);
        camera.setJpegSize(new Size(4160, 3120));
        camera.open("0");
    }

    private void stopCamera() {
        camera.close();
        surface.release();
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
