package com.game.douniuserver;

public class UserInfo {
	private int id;
	private String name;
	private String ipaddr;
	//private String logintime;
	
	public int getId() {
		return id;
	}
	
	public void setId(int id) {
		this.id = id;
	}
	
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public String getIpaddr() {
		return ipaddr;
	}
	
	public void setIpaddr(String ipaddr) {
		this.ipaddr = ipaddr;
	}
	
	//public String getLogintime() {
	//	return logintime;
	//}
	
	//public void setLogintime(String logintime) {
	//	this.logintime = logintime;
	//}
	
	@Override
	public String toString() {
		return "UserInfo [id=" + id + ", name=" + name + ", ipaddr=" + ipaddr + "]";
	}
}
