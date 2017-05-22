package com.example.muto.xxipcamera;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by muto on 17-5-22.
 */

public class XXSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "XXSurfaceView";

    public XXSurfaceView(Context context) {
        super(context);
        Log.d(TAG, "XXSurfaceView 1");
        getHolder().addCallback(this);
    }

    public XXSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        Log.d(TAG, "XXSurfaceView 2");
        getHolder().addCallback(this);
    }

    public XXSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public XXSurfaceView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        Log.d(TAG, "onAttachedToWindow");
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        Log.d(TAG, "onDetachedFromWindow");
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "surfaceCreated");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d(TAG, "surfaceChanged");
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "surfaceDestroyed");
    }
}
