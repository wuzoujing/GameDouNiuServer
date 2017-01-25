package com.game.douniuserver;

import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class UserListAdapter extends BaseAdapter {
	private String TAG = "[wzj]UserListAdapter";
	
	private Context context;
	private List<UserInfo> userInfos;
	private UserInfo userInfo;
	
	public UserListAdapter(Context context, List<UserInfo> userInfos) {
		this.context = context;
		this.userInfos = userInfos;
	}
	
	@Override
	public int getCount() {
		return userInfos.size();
	}

	@Override
	public Object getItem(int position) {
		return position;
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder viewHolder = null;
		if(convertView == null)
		{
			viewHolder = new ViewHolder();
			convertView = LayoutInflater.from(context).inflate(R.layout.userlist_item, null);
			//viewHolder.albumImage = (ImageView) convertView.findViewById(R.id.album_image);
			viewHolder.userName = (TextView) convertView.findViewById(R.id.user_name);
			viewHolder.userIP = (TextView) convertView.findViewById(R.id.user_ip);
			//viewHolder.userLoginTime = (TextView) convertView.findViewById(R.id.user_logintime);
			convertView.setTag(viewHolder);
		} else {
			viewHolder = (ViewHolder)convertView.getTag();
		}
		
		viewHolder.userName.setText(userInfo.getName());
		viewHolder.userIP.setText(userInfo.getIpaddr());
		//viewHolder.userLoginTime.setText(userInfo.getLogintime());
		
		return convertView;
	}
	
	public class ViewHolder {
		//public ImageView user_logo;
		public TextView userName;
		public TextView userIP;
		//public TextView userLoginTime;
	}
}
