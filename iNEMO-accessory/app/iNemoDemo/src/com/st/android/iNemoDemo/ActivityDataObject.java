package com.st.android.iNemoDemo;

import com.android.future.usb.UsbAccessory;
import com.st.android.iNemoDemo.iNemoInfo.INemoInfo;

public class ActivityDataObject {
	
	private INemoInfo mINemoInfo;
	private UsbAccessory mAccessory;
	
	public ActivityDataObject(){
		this.mINemoInfo = null;
		this.mAccessory = null;
	}

	public INemoInfo getmINemoInfo() {
		return mINemoInfo;
	}

	public void setmINemoInfo(INemoInfo mINemoInfo) {
		this.mINemoInfo = mINemoInfo;
	}

	public UsbAccessory getmAccessory() {
		return mAccessory;
	}

	public void setmAccessory(UsbAccessory mAccessory) {
		this.mAccessory = mAccessory;
	}
}
