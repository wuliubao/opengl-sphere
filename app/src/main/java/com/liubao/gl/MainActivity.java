package com.liubao.gl;

import android.opengl.GLSurfaceView;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private GLNativeView glNativeView;
    private GLNativeRender glNativeRender;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        glNativeView = new GLNativeView(this);
        glNativeRender =  new GLNativeRender();
        glNativeView.setEGLContextClientVersion(3);
        glNativeView.setEGLConfigChooser(8,8,8,0,16,0);

        final DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        glNativeView.setRenderer(glNativeRender, displayMetrics.density);
        setContentView(glNativeView);

    }

    @Override
    protected void onResume() {
        super.onResume();
        glNativeView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        glNativeView.onPause();
    }

}
