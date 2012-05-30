package com.st.android.iNemoDemo;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import com.android.future.usb.UsbAccessory;
import com.android.future.usb.UsbManager;
import com.st.android.iNemoDemo.iNemoInfo.INemoInfo;

public class ADKActivity extends Activity implements Runnable{

	static final String TAG = "ADK_Activity";
	
	private static final String ACTION_USB_PERMISSION = "com.st.android.adkping.action.USB_PERMISSION";
	protected static final int NO_DEVICE = 0;
	protected static final int ATTACHED_DEVICE = 1;
	protected static final int CONNECTED_DEVICE = 2;
	protected static final int ACQUISITION = 3;
	protected static final int SETTINGS = 4;

	private UsbManager mUsbManager;
	private PendingIntent mPermissionIntent;
	private boolean mPermissionRequestPending;

	UsbAccessory mAccessory;
	ParcelFileDescriptor mAccessoryFileDescriptor;
	FileInputStream mAccessoryInput;
	FileOutputStream mAccessoryOutput;

	protected INemoInfo mINemoInfo = null;
	protected int mStatus = 0;

	private final BroadcastReceiver mUsbBroadcastReceiver = new BroadcastReceiver()
	{
		public void onReceive(Context context, Intent intent)
		{
			String action = intent.getAction();
			if (ACTION_USB_PERMISSION.equals(action)){
				synchronized (this){
					UsbAccessory accessory = UsbManager.getAccessory(intent);
					if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
						openAccessory(accessory);
					} else {
						Log.d(TAG, "permission denied for accessory " + accessory);
					}
					mPermissionRequestPending = false;
				}
			}
			else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)){
				UsbAccessory accessory = UsbManager.getAccessory(intent);
				if (accessory != null && accessory.equals(mAccessory)) {
					closeAccessory();
				}
			}
		}
	};
	
	Handler mHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {			
			CommunicationFrame cf = (CommunicationFrame) msg.obj;
			handleReceivedMsg(cf);
		}
	};

	protected void setStatus(int status) {} 
	
	protected void handleReceivedMsg(CommunicationFrame cf) {}
	
	protected void resume(){
		this.setStatus(mStatus);
	}
		
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		mUsbManager = UsbManager.getInstance(this);

		/* Handle the Accessory stuff */
		mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
		IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
		filter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
		registerReceiver(mUsbBroadcastReceiver, filter);

		final ActivityDataObject obj = (ActivityDataObject)getLastNonConfigurationInstance();
		if (obj != null){
			// TODO seguita la documentazione ma non funziona (???) 
			this.mAccessory = obj.getmAccessory();
			this.mINemoInfo = obj.getmINemoInfo();
			if(this.mAccessory != null){
				openAccessory(this.mAccessory);
			}
		}
		
		this.setStatus(mStatus);
	}


	@Override
	public Object onRetainNonConfigurationInstance()
	{
		final ActivityDataObject obj = new ActivityDataObject();
		obj.setmAccessory(mAccessory);
		obj.setmINemoInfo(mINemoInfo);
		return obj;
	}
	
	@Override
	public void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);
		mStatus = savedInstanceState.getInt("status");
	}
	
	protected void onSaveInstanceState(Bundle savedInstanceState) {
		super.onSaveInstanceState(savedInstanceState);
		savedInstanceState.putInt("status", mStatus);
	}

	@Override
	public void onResume()
	{
		super.onResume();
		
		if(mAccessory == null)
			checkConnectedAccesory();
		
		this.setStatus(mStatus);
	}

	@Override
	public void onPause()
	{
		super.onPause();
		
	}

	@Override
	public void onDestroy()
	{
		closeAccessory();
		unregisterReceiver(mUsbBroadcastReceiver);
		super.onDestroy();
	}

	private void checkConnectedAccesory(){
		UsbAccessory[] accessories = mUsbManager.getAccessoryList();
		UsbAccessory accessory = (accessories == null ? null : accessories[0]);
		if (accessory != null) {
			if (mUsbManager.hasPermission(accessory)) {
				openAccessory(accessory);
			} else {
				synchronized (mUsbBroadcastReceiver) {
					if (!mPermissionRequestPending) {
						mUsbManager.requestPermission(accessory, mPermissionIntent);
						mPermissionRequestPending = true;
					}
				}
			}
		} else {
			Log.e(TAG, "mAccessory is null");
		}
	}

	private void openAccessory(UsbAccessory accessory)
	{
		mAccessoryFileDescriptor = mUsbManager.openAccessory(accessory);
		if (mAccessoryFileDescriptor != null)
		{
			this.mAccessory = accessory;
			FileDescriptor fd = mAccessoryFileDescriptor.getFileDescriptor();
			mAccessoryInput = new FileInputStream(fd);
			mAccessoryOutput = new FileOutputStream(fd);
			Thread thread = new Thread(null, this, "ADKThread");
			thread.start();
			setStatus(ATTACHED_DEVICE);			
			Log.d(TAG, "accessory opened");
		}
		else{
			Log.d(TAG, "accessory open fail");
		}
	}

	private void closeAccessory()
	{
		try{
			if(mAccessoryFileDescriptor != null)
				mAccessoryFileDescriptor.close();
		}
		catch (IOException e)
		{}
		finally{
			mAccessoryFileDescriptor = null;
			mAccessory = null;
			setStatus(NO_DEVICE);
		}
	}
	
	public void run() {
		int ret = 0;
		byte[] buffer = new byte[16384];

		while (ret >= 0) {
			try {
				ret = mAccessoryInput.read(buffer);
			} catch (IOException e) {
				break;
			}
			
			CommunicationFrame cf = new CommunicationFrame();
			Message m = Message.obtain();
			cf.setMsg(buffer);
			m.obj = cf;
			mHandler.sendMessage(m);
		}
	}

	public void sendCommand(CommunicationFrame cf) {		
		if (mAccessoryOutput != null) {
			try {
				mAccessoryOutput.write(cf.getMsg());
			} catch (IOException e) {
				Log.e(TAG, "write failed", e);
			}
		}
	}
}
