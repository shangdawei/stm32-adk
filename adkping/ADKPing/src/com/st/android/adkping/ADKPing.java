package com.st.android.adkping;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.text.method.ScrollingMovementMethod;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.future.usb.UsbAccessory;
import com.android.future.usb.UsbManager;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

public class ADKPing extends Activity implements Runnable
{
	private final Logger logger = LoggerFactory.getLogger("ADKPing");

	private UsbManager usbManager;

	UsbAccessory accessory;
	ParcelFileDescriptor accessoryFileDescriptor;
	FileInputStream accessoryInput;
	FileOutputStream accessoryOutput;

	LinearLayout layout;
	TextView logTextView;
	
	Handler mHandler;
  
	private final BroadcastReceiver usbBroadcastReceiver = new BroadcastReceiver()
	{
		public void onReceive(Context context, Intent intent)
		{
			String action = intent.getAction();
			if (UsbManager.ACTION_USB_ACCESSORY_ATTACHED.equals(action)){
				synchronized (this){
				 accessory = UsbManager.getAccessory(intent);
				}
			}
			else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)){
				UsbAccessory accessory = UsbManager.getAccessory(intent);
				if (accessory != null)
				{
				 // call your method that cleans up and closes communication with the accessory
				}
			}
		}
	};


	void sendText(String text)
	{
		Message msg = new Message();
		msg.obj = text;
		mHandler.sendMessage(msg);
	}
	
	/**
	 * Main USB reading loop, processing incoming data from accessory
	 */
	public void run()
	{
		int ret = 0;
		byte[] buffer = new byte[16384];

		while(true){
			try{
				sendText("Receiving data\n");
			
				ret = accessoryInput.read(buffer);

				sendText("Received" + ret + "bytes\n");

				buffer[0] = 'c';
				buffer[1] = 'i';
				buffer[2] = 'a';
				buffer[3] = 'o';
        
				sendText("Sending back data\n");
				
				accessoryOutput.write(buffer, 0, ret);
				
				sendText("Sent\n");
			}
			catch (IOException e){
				sendText("Exception in reading/writing accessory\n");
				logger.debug("Exception in USB accessory input reading", e);
			}
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		usbManager = UsbManager.getInstance(this);

		/* Handle the Accessory stuff */
		IntentFilter filter = new IntentFilter(UsbManager.ACTION_USB_ACCESSORY_ATTACHED);
		filter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
		registerReceiver(usbBroadcastReceiver, filter);

		if (getLastNonConfigurationInstance() != null){
			accessory = (UsbAccessory) getLastNonConfigurationInstance();
			openAccessory(accessory);
		}
    
		/* Set up the UI */	
		setContentView(R.layout.main);
		logTextView = (TextView)findViewById(R.id.logTextView);
		logTextView.setMovementMethod(new ScrollingMovementMethod());
		
		/* Set up message handler used to update the UI from different thread 
		 * which would crash UI with "CalledFromWrongThreadException" otherwise
		 */
		mHandler = new Handler()
		{
			@Override
			public void handleMessage(Message msg)
				{
				String text = (String)msg.obj;
				logTextView.append(text);
				//msg.recycle();
				}
		};
	}

	
	@Override
	public Object onRetainNonConfigurationInstance()
	{
		return accessory != null ? accessory : super.onRetainNonConfigurationInstance();
	}

	@Override
	public void onResume()
	{
		super.onResume();

		Intent intent = getIntent();
		if (accessoryInput != null && accessoryOutput != null)
			return;

		// TODO: verify, docs don't do this simple thing, not sure why?
		UsbAccessory accessory = UsbManager.getAccessory(intent);
		if (accessory != null)
			openAccessory(accessory);
		else
			logger.error("Failed to resume accessory.");
	}

  @Override
  public void onPause()
  {
    super.onPause();
    closeAccessory();//daz dont we need to set accessory null ?
  }

  @Override
  public void onDestroy()
  {
    unregisterReceiver(usbBroadcastReceiver);
    super.onDestroy();
  }

  private void openAccessory(UsbAccessory accessory)
  {
    accessoryFileDescriptor = usbManager.openAccessory(accessory);
    if (accessoryFileDescriptor != null)
    {
      this.accessory = accessory;
      FileDescriptor fd = accessoryFileDescriptor.getFileDescriptor();
      accessoryInput = new FileInputStream(fd);
      accessoryOutput = new FileOutputStream(fd);
      Thread thread = new Thread(null, this, "ADKPingThread");
      thread.start();
      logTextView.append("Accessory Opened\n");
      logger.debug("accessory opened");
      // TODO: enable USB operations in the app
    }
    else{
    	logTextView.append("Accessory failed\n");
    	logger.debug("accessory open fail");
    }
  }

  private void closeAccessory()
  {
    // TODO: disable USB operations in the app
    try{
      if(accessoryFileDescriptor != null)
        accessoryFileDescriptor.close();
    }
    catch (IOException e)
    {}
    finally{
      accessoryFileDescriptor = null;
      accessory = null;
    }
  }
}
