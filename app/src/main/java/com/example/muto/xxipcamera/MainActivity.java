package com.example.muto.xxipcamera;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraManager;
import android.media.Image;
import android.media.ImageReader;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.mut0.xxcam.XXCamera;

import junit.framework.Assert;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    XXCamera cam0, cam1;
    int mCurrentCamera = 0;

    private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED
                || ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                    this,
                    new String[]{Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE},
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
        Assert.assertEquals(grantResults.length, 2);
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            startCamera();
        }
    }

    private void startCamera() {
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        {
            SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
            SurfaceHolder holder = surfaceView.getHolder();
            cam0 = new XXCamera(manager, holder, mJpegListener);
            cam0.setJpegSize(new Size(4160,3120));
//            cam0.open("0");
        }
        {
            SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView2);
            SurfaceHolder holder =surfaceView.getHolder();
            cam1 = new XXCamera(manager, holder, mJpegListener);
            cam1.setJpegSize(new Size(1600, 1200));
        }
        cam0.open("0");
        mCurrentCamera = 0;

        Button btnCapture = (Button) findViewById(R.id.btnCapture);
        btnCapture.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (mCurrentCamera == 0) {
                            cam0.takePicture();
                        }
                        else {
                            cam1.takePicture();
                        }
                    }
                }
        );
        Button btnSwitch = (Button) findViewById(R.id.btnSwitch);
        btnSwitch.setOnClickListener(
                new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        if (mCurrentCamera == 0) {
                            cam0.close();
                            cam1.open("2");
                            mCurrentCamera = 1;
                        }
                        else {
                            cam1.close();
                            cam0.open("0");
                            mCurrentCamera = 0;
                        }
                    }
                }
        );
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }


    private ImageReader.OnImageAvailableListener mJpegListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireNextImage();
            boolean success = false;
            File file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM),"JPEG_" + System.currentTimeMillis() + ".jpg");
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
                MediaScannerConnection.scanFile(getApplicationContext(), new String[]{file.getPath()},
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
                Toast.makeText(getApplicationContext(), (String) msg.obj, Toast.LENGTH_SHORT).show();
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
    };
}
