package com.game.douniuserver;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

import com.game.douniuserver.jni.DouniuServerInterface;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

public class MainActivity extends Activity implements OnClickListener {
	private static String TAG = "[wzj]MainActivity";
	
	private DouniuService serviceBinder;
	
	private Button startServerBtn;
	private Button stopServerBtn;
	private ListView usersLv;
	private TextView statusTv;
	private TextView localIPTv;
	
	private UserListAdapter adapter;
	private List<UserInfo> userInfos = new ArrayList<UserInfo>();
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		startServerBtn = (Button)findViewById(R.id.start_server_btn);
		stopServerBtn = (Button)findViewById(R.id.stop_server_btn);
		startServerBtn.setOnClickListener(this);
		stopServerBtn.setOnClickListener(this);
		usersLv = (ListView)findViewById(R.id.users_lv);
		statusTv = (TextView)findViewById(R.id.status_tv);
		localIPTv = (TextView)findViewById(R.id.localip_tv);

		adapter = new UserListAdapter(this, userInfos);
		usersLv.setAdapter(adapter);
		
		getIPAddr();
	}

	@Override
	protected void onStart() {
		Log.v(TAG, "onStart");
		super.onStart();
		//IntentFilter filter = new IntentFilter();
		//filter.addAction(Constants.ACTION_SERVER_STATUS);
		//filter.addAction(Constants.ACTION_USER_LOGIN);
		//registerReceiver(statusReceiver, filter);
	}

	@Override
	protected void onResume() {
		Log.v(TAG, "onResume");
		super.onResume();
	}

	@Override
	protected void onPause() {
		Log.v(TAG, "onPause");
		super.onPause();
	}

	@Override
	protected void onStop() {
		Log.v(TAG, "onStop");
		super.onStop();
		//unregisterReceiver(statusReceiver);
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.start_server_btn:
			Log.v(TAG, "start_server_btn");
			startServerAction();
			//getIPAddr();
			//Intent bindIntent = new Intent(MainActivity.this, DouniuService.class);  
	        //bindService(bindIntent, sconnection, Context.BIND_AUTO_CREATE);  
			break;
		case R.id.stop_server_btn:
			Log.v(TAG, "stop_server_btn");
			stopServerAction();
			//unbindService(sconnection);
			//statusTv.setText("service not running");
			break;
		default:
			break;
		}
	}
	
	public void startServerAction() {
		Log.v(TAG, "startServerAction");
		getIPAddr();
		Intent bindIntent = new Intent(MainActivity.this, DouniuService.class);  
        bindService(bindIntent, sconnection, Context.BIND_AUTO_CREATE);  
	}
	
	public void stopServerAction() {
		Log.v(TAG, "stopServerAction");
		try {
			unbindService(sconnection);
		}
		catch (IllegalArgumentException e) {
			Log.v(TAG, "stop Server exception.");
			e.printStackTrace();
		}
		statusTv.setText("service not running");
	}
	
    public void getIPAddr() {
        //��ȡwifi����  
        WifiManager wifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);  
        //�ж�wifi�Ƿ���  
        if (!wifiManager.isWifiEnabled()) {  
        	wifiManager.setWifiEnabled(true);    
        }  
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();       
        int ipAddress = wifiInfo.getIpAddress();   
        String ip = intToIp(ipAddress);   
        localIPTv.setText("ip:"+ip);
    }
	
    private String intToIp(int i) {       
        return (i & 0xFF ) + "." +       
        		((i >> 8 ) & 0xFF) + "." +       
        		((i >> 16 ) & 0xFF) + "." +       
        		( i >> 24 & 0xFF) ;  
    }
    
	ServiceConnection sconnection = new ServiceConnection() {  
        /*����ʱִ��*/
		@Override
        public void onServiceConnected(ComponentName name, IBinder service) {  
            Log.d(TAG, "onServiceConnected");  
            Intent intent = new Intent();//����ֻ��Ϊ�����洫intent����������ģ�û��ʵ������  
            /*�󶨺�Ϳ���ʹ��Service����ط�������������ʼ���Service�Ĳ���*/  
            serviceBinder = ((DouniuService.MyBinder) service).getService();  
            /*���磺����Ե���Service��onStartCommand()����*/  
            statusTv.setText("service running");
            serviceBinder.onStartCommand(intent, 0, 0);//0,0��������Ĳ���  
        }
        
        /*��������ĶϿ�����ʱִ�У�������unbindService()ʱ���ᴥ���ķ���*/  
        @Override
        public void onServiceDisconnected(ComponentName name) {  
            Log.d(TAG, "onServiceDisconnected");
            //statusTv.setText("service not running");
        }
    };
    
	protected BroadcastReceiver statusReceiver = new BroadcastReceiver() {
		
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			//Log.v(TAG, "[onReceive]action:"+action);
			if (action.equals(Constants.ACTION_SERVER_STATUS)){
				Log.v(TAG, "[onReceive]ACTION_SERVER_STATUS");
				int serverStatus =  intent.getExtras().getInt(Constants.PARA_SERVER_STATUS);
				if (serverStatus == Constants.SERVER_STATUS_CONNECTED) {
					statusTv.setText("Game server is running");
				} else if (serverStatus == Constants.SERVER_STATUS_DISCONNECTED) {
					statusTv.setText("Game server is not running");
				}
			} else if (action.equals(Constants.ACTION_USER_LOGIN)){
				Log.v(TAG, "[onReceive]ACTION_USER_LOGIN");
			}
		}
	};
}
