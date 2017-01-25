package com.game.douniuserver.jni;


import android.util.Log;


public class DouniuServerInterface {
	private static String TAG = "[wzj]DouniuServerInterface";

	static {

		try {
			System.loadLibrary("douniuserver_jni");
		} catch (Throwable ex) {
			// ex.printStackTrace();
			Log.e(TAG, "load douniuserver_jni error!");
		}
	}
	
	public DouniuServerInterface() {
	}
	
	public String getString() {
		Log.d(TAG, "[getString]->stringFromJNI");
		String str = stringFromJNI();
		Log.d(TAG, "[getString]str:"+str);
		return str;
	}
	
	public int initAndAccept() {
		Log.d(TAG, "[initAndAccept]->nativeInitAndAccept");
		int ret = nativeInitAndAccept();
		Log.d(TAG, "[initAndAccept]ret:"+ret);
		return ret;
	}
	
	public void disconnected() {
		Log.d(TAG, "[disconnected]->nativeDisconnected");
		nativeDisconnected();
	}

	public native String stringFromJNI();
	public native int nativeInitAndAccept();
	public native void nativeDisconnected();
}
