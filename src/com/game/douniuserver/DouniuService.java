package com.game.douniuserver;

import com.game.douniuserver.jni.DouniuServerInterface;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

public class DouniuService extends Service {
	private final String TAG = "[wzj]DouniuService";
	
	private final IBinder binder = new MyBinder();
	private DouniuServerInterface douniuServer;
	private Handler handler = new Handler();
	
	@Override
	public IBinder onBind(Intent intent) {
		Log.d(TAG, "onBind");
		return binder;
	}
	
	@Override
	public void onCreate() {
		Log.d(TAG, "onCreate");
		douniuServer = new DouniuServerInterface();
		super.onCreate();
	}

	@Override
	public void onDestroy() {
		Log.d(TAG, "onDestroy");
		super.onDestroy();
	}

	@Override
	public boolean onUnbind(Intent intent) {
		Log.d(TAG, "onUnbind");
		Thread thread2 = new Thread(doDisconnected, "thread2");
		thread2.start();
		return super.onUnbind(intent);
	}

	@Override
	public void onRebind(Intent intent) {
		Log.d(TAG, "onRebind");
		super.onRebind(intent);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.d(TAG, "onStartCommand flags=" + flags + "  startId=" + startId); 
		//douniuServer.initAndAccept();
		/*if (douniuServer.initAndAccept() == 1) {//OK
			Log.d(TAG, "initAndAccept success");
			handler.postDelayed(new Runnable() {
				
				@Override
				public void run() {
					Intent intent2 = new Intent();
					intent2.setAction(Constants.ACTION_SERVER_STATUS);
					intent2.putExtra(Constants.PARA_SERVER_STATUS, Constants.SERVER_STATUS_CONNECTED);
					sendBroadcast(intent2);
				}
			}, 100);
		} else {
			Log.d(TAG, "initAndAccept fail");
			handler.postDelayed(new Runnable() {
				
				@Override
				public void run() {
					Intent intent2 = new Intent();
					intent2.setAction(Constants.ACTION_SERVER_STATUS);
					intent2.putExtra(Constants.PARA_SERVER_STATUS, Constants.SERVER_STATUS_DISCONNECTED);
					sendBroadcast(intent2);
				}
			}, 100);
		}*/
		Thread thread1 = new Thread(doInitAndAccept, "thread1");
		thread1.start();
		Log.d(TAG, "onStartCommand end");
		return super.onStartCommand(intent, flags, startId);
	}
	
	private Runnable doInitAndAccept = new Runnable() {
		
		@Override
		public void run() {
			Log.d(TAG, "[run]->initAndAccept");
			int ret = douniuServer.initAndAccept();
			if (ret == 1) {
				Log.d(TAG, "initAndAccept success");
				Intent intent2 = new Intent();
				intent2.setAction(Constants.ACTION_SERVER_STATUS);
				intent2.putExtra(Constants.PARA_SERVER_STATUS, Constants.SERVER_STATUS_CONNECTED);
				sendBroadcast(intent2);
			} else {
				Log.d(TAG, "initAndAccept fail");
				Intent intent2 = new Intent();
				intent2.setAction(Constants.ACTION_SERVER_STATUS);
				intent2.putExtra(Constants.PARA_SERVER_STATUS, Constants.SERVER_STATUS_DISCONNECTED);
				sendBroadcast(intent2);
			}
		}
	};
	
	private Runnable doDisconnected = new Runnable() {
		
		@Override
		public void run() {
			Log.d(TAG, "[run]->disconnected");
			douniuServer.disconnected();
		}
	};


	public class MyBinder extends Binder {  
		DouniuService getService() {
            return DouniuService.this;
        }
    }
}
