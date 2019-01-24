package com.liubao.gl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.MotionEvent;

/**
 * Create By Liubao
 * On 2019/1/20 0020
 */
public class GLNativeView extends GLSurfaceView {

    private float preX;
    private float preY;
    private float touchDensity;

    private Dispatcher dispatcher;

    public GLNativeView(Context context){
        super(context);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event != null) {
            float x = event.getX();
            float y = event.getY();

            if (event.getAction() == MotionEvent.ACTION_MOVE) {
                if (dispatcher != null) {
                    float deltaX = (x - preX) / touchDensity ;
                    float deltaY = (y - preY) / touchDensity ;
                    dispatcher.dispatchDelta(deltaX, deltaY);

                }
            }

            preX = x;
            preY = y;

            return true;
        } else {
            return super.onTouchEvent(event);
        }
    }

    public void setRenderer(Renderer renderer, float density) {
        dispatcher = (Dispatcher) renderer;
        touchDensity = density;
        super.setRenderer(renderer);
    }

}
