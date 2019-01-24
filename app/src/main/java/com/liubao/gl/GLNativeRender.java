package com.liubao.gl;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Create By Liubao
 * On 2019/1/20 0020
 */
public class GLNativeRender implements GLSurfaceView.Renderer, Dispatcher {
    static {
        System.loadLibrary("native-lib");
    }

    public static native boolean nativeInit();

    public static native void nativeDrawFrame();

    public static native void nativeResize(int width, int height);

    public static native void nativeDispatchEvent(float x, float y);

    public void dispathEvent(float deltaX, float deltaY) {
        nativeDispatchEvent(deltaX, deltaY);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit();
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        nativeDrawFrame();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height){
        nativeResize(width, height);
    }

    @Override
    public void dispatchDelta(float deltaX, float deltaY) {
        nativeDispatchEvent(deltaX, deltaY);
    }

}
