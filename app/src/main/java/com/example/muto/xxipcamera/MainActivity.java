package com.example.muto.xxipcamera;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraManager;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import com.mut0.xxcam2.XXAEncoder;
import com.mut0.xxcam2.XXCamera2;
import com.mut0.xxcam2.XXFlvMux;
import com.mut0.xxcam2.XXMicroPhone;

import junit.framework.Assert;

public class MainActivity extends AppCompatActivity {

    XXCamera2 cam0,cam1;
    private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    this,
                    new String[] { Manifest.permission.CAMERA },
                    PERMISSION_REQUEST_CODE_CAMERA);
            return;
        }

        startCamera();
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
        Assert.assertEquals(grantResults.length, 1);
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            startCamera();
        }
    }

    private void startCamera()
    {
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        {
            SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
            SurfaceHolder holder =surfaceView.getHolder();
            holder.setFixedSize(1280,720);
            cam0 = new XXCamera2(manager, holder);
            cam0.init("0");
        }
//        {
//            SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView2);
//            SurfaceHolder holder =surfaceView.getHolder();
//            holder.setFixedSize(1280,720);
//            cam1 = new XXCamera2(manager, holder);
//            cam1.init("2");
//        }

        Button btn = (Button) findViewById(R.id.button);
        btn.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
//                        XXFlvMux.nativeTest();
//                        XXAEncoder xp = new XXAEncoder();
                        cam0.takePicture();
                    }
                }
        );
    }
    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }


    @Override
    protected void onResume() {
        super.onResume();
    }


    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }



}
