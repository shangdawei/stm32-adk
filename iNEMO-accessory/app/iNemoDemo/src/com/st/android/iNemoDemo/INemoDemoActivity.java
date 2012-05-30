package com.st.android.iNemoDemo;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.ArrayList;

import com.st.android.iNemoDemo.R;
import com.st.android.iNemoDemo.iNemoInfo.INemoInfo;
import com.st.android.iNemoDemo.iNemoInfo.Sensor;

import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class INemoDemoActivity extends ADKActivity {
	
	public int SETTINGS_ACTIVITY_REQUEST = 0;
		
	protected void setStatus(int status){
		this.mStatus = status;
		if(status == ATTACHED_DEVICE){
			TextView deviceLabel;
			Button connectButton;
			setContentView(R.layout.connect_device);
			deviceLabel = (TextView)findViewById(R.id.deviceLabel);
			connectButton = (Button)findViewById(R.id.connectButton);
			if(connectButton != null){
				connectButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {			
						CommunicationFrame cf = new CommunicationFrame();
						cf.setConnectFrame();
						sendCommand(cf);					
					}
				});
				connectButton.setEnabled(true);
			}
			if(deviceLabel != null)
				deviceLabel.setText("Detected Device: " + this.mAccessory.getManufacturer() + "\n");
			
		} else if(status == NO_DEVICE){
			setContentView(R.layout.no_device);
		} else if(status == CONNECTED_DEVICE){
			CommunicationFrame 	cf = new CommunicationFrame();
			Button disconnectButton, settingsButton, startAcquisitionButton;
			setContentView(R.layout.connected_dev);	
			if(this.mINemoInfo == null){
				this.mINemoInfo = new INemoInfo();
				cf.setGetMCUIDFrame();
				sendCommand(cf);
				cf.setGetFWVersionFrame();
				sendCommand(cf);
				cf.setGetHWVersionFrame();
				sendCommand(cf);
				cf.setGetOutputModeFrame();
				sendCommand(cf);
			}
			else
				this.updateConnectedLayout();
			disconnectButton = (Button)findViewById(R.id.disconnectButton);
			settingsButton = (Button)findViewById(R.id.settingsButton);
			startAcquisitionButton = (Button)findViewById(R.id.startAcquisitionButton);
			if(disconnectButton != null){
				disconnectButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {			
						CommunicationFrame cf = new CommunicationFrame();
						cf.setDisconnectFrame();
						sendCommand(cf);					
					}
				});
			}
			if(settingsButton != null){
				settingsButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {			
						startSettingsActivity();						
					}
				});
			}
			if(startAcquisitionButton != null){
				startAcquisitionButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {			
						CommunicationFrame cf = new CommunicationFrame();
						cf.setStartAcquisitionFrame();
						sendCommand(cf);			
					}
				});
			}
		} else if(status == ACQUISITION){
			Button stopAcquisitionButton;
			setContentView(R.layout.acquisition);
			stopAcquisitionButton = (Button)findViewById(R.id.stopAcquisitionButton);
			if(stopAcquisitionButton != null){
				stopAcquisitionButton.setOnClickListener(new OnClickListener() {
					@Override
					public void onClick(View v) {			
						CommunicationFrame cf = new CommunicationFrame();
						cf.setStopAcquisitionFrame();
						sendCommand(cf);					
					}
				});
			}
		}
	}
	
	protected void handleReceivedMsg(CommunicationFrame cf){
		if(cf.isConnectACK()){
			this.setStatus(CONNECTED_DEVICE);
		} else if(cf.isConnectNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in connecting device: " + cf.getErrorCode() + "\n");
		} else if(cf.isDisconnectACK()){
			this.mINemoInfo = null;
			this.setStatus(ATTACHED_DEVICE);
		} else if(cf.isDisconnectNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in disconnecting device: " + cf.getErrorCode() + "\n");
		} else if(cf.isGetMCUIDACK()){
			if(this.mINemoInfo != null){
				this.mINemoInfo.setUID(cf.getStringPayload());
				updateConnectedLayout();
			}
		} else if(cf.isGetMCUIDNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in getting device UID: " + cf.getErrorCode() + "\n");
		} else if(cf.isGetFWVersionACK()){
			if(this.mINemoInfo != null){
				this.mINemoInfo.setFW_v(cf.getStringPayload());
				updateConnectedLayout();
			}
		} else if(cf.isGetFWVersionNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in getting FW info: " + cf.getErrorCode() + "\n");
		} else if(cf.isGetHWVersionACK()){
			if(this.mINemoInfo != null){
				this.mINemoInfo.setHW_v(cf.getStringPayload());
				updateConnectedLayout();
			}
		} else if(cf.isGetHWVersionNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in getting HW info: " + cf.getErrorCode() + "\n");
		} else if(cf.isSetSensorParameterACK()){
			updateConnectedLayout();
		} else if(cf.isSetSensorParameterNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in setting parameter: " + cf.getErrorCode() + "\n");
		} else if(cf.isGetSensorParameterACK()){
			if(cf.updateParameter(this.mINemoInfo))
				updateConnectedLayout();
			else {
				TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
				if(debugLabel != null)
				debugLabel.append("Error in getting parameter value\n");			
			}
		} else if(cf.isGetSensorParameterNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in getting parameter value: " + cf.getErrorCode() + "\n");
		} else if(cf.isRestoreDefaultParameterACK()){
			if(cf.updateParameter(this.mINemoInfo))
				updateConnectedLayout();
			else {
				TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
				if(debugLabel != null)
				debugLabel.append("Error in restoring parameter value\n");			
			}
		} else if(cf.isRestoreDefaultParameterNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in restoring parameter value: " + cf.getErrorCode() + "\n");
		} else if(cf.isSetOutputModeACK()){
			updateConnectedLayout();
		} else if(cf.isSetOutputModeNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in setting output mode: " + cf.getErrorCode() + "\n");
		} else if(cf.isGetOutputModeACK()){
			SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
			if(cf.updateOutputMode(mINemoInfo, sharedPrefs)){
				updateConnectedLayout();
				for(Byte b : mINemoInfo.getSensors().keySet()){
					Sensor s = mINemoInfo.getSensors().get(b);
					if(s.isEnabled()){
						for(Byte b2 : s.getParameters().keySet()){
							CommunicationFrame newCf = new CommunicationFrame();
							newCf.setGetSensorParameterFrame(b, b2);
							sendCommand(newCf);
						}
					}
				}
			} else {
				TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
				if(debugLabel != null)
				debugLabel.append("Error in updating output mode\n");			
			}
		} else if(cf.isGetOutputModeNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in getting output mode: " + cf.getErrorCode() + "\n");
		} else if(cf.isStartAcquisitonACK()){
			this.setStatus(ACQUISITION);
		} else if(cf.isStartAcquisitonNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in starting acquisition: " + cf.getErrorCode() + "\n");
		} else if(cf.isStopAcquisitonACK()){
			this.setStatus(CONNECTED_DEVICE);
		} else if(cf.isStopAcquisitonNACK()){
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Error in stopping acquisition: " + cf.getErrorCode() + "\n");
		} else if(cf.isAcquisitionData()){
			if(this.mINemoInfo != null){
				cf.updateAcquiredData(mINemoInfo);
				TextView dataLabel = (TextView)findViewById(R.id.dataLabel);
				if(dataLabel != null)
					dataLabel.setText(this.mINemoInfo.printAcquisitionData());
			}
		} else{
			TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
			if(debugLabel != null)
				debugLabel.append("Unknown message received\n");
		}
		
	}
	
	private void updateConnectedLayout(){
		TextView iNemoInfolabel = (TextView)findViewById(R.id.iNemoInfolabel);
		if(iNemoInfolabel != null && this.mINemoInfo != null)
			iNemoInfolabel.setText(this.mINemoInfo.toString());
	}
	
	public void startSettingsActivity(){
		Intent settingsActivity = new Intent(this, SettingsActivity.class);
		startActivity(settingsActivity);
	}
	
	public void onResume(){
		super.onResume();
		
		SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
		if(sharedPrefs != null && this.mINemoInfo != null && this.mStatus == CONNECTED_DEVICE){
			try{	
				boolean b = false, changedOutMode = false;
				String s = "";
				byte by = 0;
				byte[] by2;
				byte[] by3;
				ArrayList<CommunicationFrame> cfs = new ArrayList<CommunicationFrame>();
						
				b = sharedPrefs.getBoolean("calibrated_data", Boolean.getBoolean(getString(R.string.def_calibrated_data)));
				if(this.mINemoInfo.isCalibratedData() != b){
					this.mINemoInfo.setCalibratedData(b);
					changedOutMode = true;
				}							
				s = sharedPrefs.getString("acquisition_rate", getString(R.string.def_acquisition_rate));
				by = Byte.parseByte(s);
				if(this.mINemoInfo.getAcquisitionRate() != by){
					this.mINemoInfo.setAcquisitionRate(by);
					changedOutMode = true;
				}								
				b = sharedPrefs.getBoolean("ahrs_lib", Boolean.getBoolean(getString(R.string.def_ahrs_lib)));
				if(this.mINemoInfo.isAhrsLib() != b){
					this.mINemoInfo.setAhrsLib(b);
					changedOutMode = true;
				}
				b = sharedPrefs.getBoolean("enable_acc", Boolean.getBoolean(getString(R.string.def_acc)));
				if(this.mINemoInfo.isSensorEnabled(CommunicationFrame.ACCELEROMETER) != b){
					this.mINemoInfo.enableSensor(CommunicationFrame.ACCELEROMETER, b);
					changedOutMode = true;
				}
				b = sharedPrefs.getBoolean("enable_mag", Boolean.getBoolean(getString(R.string.def_mag)));
				if(this.mINemoInfo.isSensorEnabled(CommunicationFrame.MAGNETOMETER) != b){
					this.mINemoInfo.enableSensor(CommunicationFrame.MAGNETOMETER, b);
					changedOutMode = true;
				}
				b = sharedPrefs.getBoolean("enable_gyro", Boolean.getBoolean(getString(R.string.def_gyro)));
				if(this.mINemoInfo.isSensorEnabled(CommunicationFrame.GYRO_1AXIS) != b){
					this.mINemoInfo.enableSensor(CommunicationFrame.GYRO_1AXIS, b);
					this.mINemoInfo.enableSensor(CommunicationFrame.GYRO_2AXIS, b);
					changedOutMode = true;
				}
				b = sharedPrefs.getBoolean("enable_press", Boolean.getBoolean(getString(R.string.def_press)));
				if(this.mINemoInfo.isSensorEnabled(CommunicationFrame.PRESSURE) != b){
					this.mINemoInfo.enableSensor(CommunicationFrame.PRESSURE, b);
					changedOutMode = true;
				}
				b = sharedPrefs.getBoolean("enable_temp", Boolean.getBoolean(getString(R.string.def_temp)));
				if(this.mINemoInfo.isSensorEnabled(CommunicationFrame.TEMPERATURE) != b){
					this.mINemoInfo.enableSensor(CommunicationFrame.TEMPERATURE, b);
					changedOutMode = true;
				}
				if(changedOutMode){
					CommunicationFrame cf = new CommunicationFrame();
					cf.setSetOutputModeFrame(mINemoInfo.isSensorEnabled(CommunicationFrame.TEMPERATURE),
							mINemoInfo.isSensorEnabled(CommunicationFrame.PRESSURE),
							mINemoInfo.isSensorEnabled(CommunicationFrame.MAGNETOMETER),
							mINemoInfo.isSensorEnabled(CommunicationFrame.GYRO_1AXIS),
							mINemoInfo.isSensorEnabled(CommunicationFrame.ACCELEROMETER),
							mINemoInfo.isCalibratedData(), mINemoInfo.isAhrsLib(), mINemoInfo.getAcquisitionRate());
					cfs.add(cf);
				}
								
				s = sharedPrefs.getString("acc_out_rate", getString(R.string.def_acc_out_rate));
				by = Byte.parseByte(s);
				by2 = this.mINemoInfo.getSensorParameterValue(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OUT_RATE);
				if(by2[0] != by){
					CommunicationFrame cf = new CommunicationFrame();
					byte[] temp_b = new byte[1];
					temp_b[0] = by;
					this.mINemoInfo.updateSensorParameter(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OUT_RATE, temp_b);
					cf.setSetSensorParameterFrame(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OUT_RATE, temp_b);
					cfs.add(cf);
				}				
				s = sharedPrefs.getString("acc_full_scale", getString(R.string.def_acc_full_scale));
				by = Byte.parseByte(s);
				by2 = this.mINemoInfo.getSensorParameterValue(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_FULL_SCALE);
				if(by2[0] != by){
					CommunicationFrame cf = new CommunicationFrame();
					byte[] temp_b = new byte[1];
					temp_b[0] = by;
					this.mINemoInfo.updateSensorParameter(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_FULL_SCALE, temp_b);
					cf.setSetSensorParameterFrame(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_FULL_SCALE, temp_b);
					cfs.add(cf);
				}
				s = sharedPrefs.getString("acc_offset_x", getString(R.string.def_acc_off_x));
				by3 = CommunicationFrame.convertShortInBytes(Short.parseShort(s));
				by2 = this.mINemoInfo.getSensorParameterValue(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_X);
				if(by2[0] != by3[0] || by2[1] != by3[1]){
					CommunicationFrame cf = new CommunicationFrame();
					this.mINemoInfo.updateSensorParameter(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_X, by3);
					cf.setSetSensorParameterFrame(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_X, by3);
					cfs.add(cf);
				}
				s = sharedPrefs.getString("acc_offset_y", getString(R.string.def_acc_off_y));
				by3 = CommunicationFrame.convertShortInBytes(Short.parseShort(s));
				by2 = this.mINemoInfo.getSensorParameterValue(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_Y);
				if(by2[0] != by3[0] || by2[1] != by3[1]){
					CommunicationFrame cf = new CommunicationFrame();
					this.mINemoInfo.updateSensorParameter(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_Y, by3);
					cf.setSetSensorParameterFrame(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_Y, by3);
					cfs.add(cf);
				}
				s = sharedPrefs.getString("acc_offset_z", getString(R.string.def_acc_off_z));
				by3 = CommunicationFrame.convertShortInBytes(Short.parseShort(s));
				by2 = this.mINemoInfo.getSensorParameterValue(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_Z);
				if(by2[0] != by3[0] || by2[1] != by3[1]){
					CommunicationFrame cf = new CommunicationFrame();
					this.mINemoInfo.updateSensorParameter(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_Z, by3);
					cf.setSetSensorParameterFrame(CommunicationFrame.ACCELEROMETER, CommunicationFrame.ACC_OFFSET_Z, by3);
					cfs.add(cf);
				}
					
				s = sharedPrefs.getString("temp_offset", getString(R.string.def_temp_off));
				by3 = CommunicationFrame.convertShortInBytes(Short.parseShort(s));
				by2 = this.mINemoInfo.getSensorParameterValue(CommunicationFrame.TEMPERATURE, CommunicationFrame.TEMP_OFFSET);
				if(by2[0] != by3[0] || by2[1] != by3[1]){
					CommunicationFrame cf = new CommunicationFrame();
					this.mINemoInfo.updateSensorParameter(CommunicationFrame.TEMPERATURE, CommunicationFrame.TEMP_OFFSET, by3);
					cf.setSetSensorParameterFrame(CommunicationFrame.TEMPERATURE, CommunicationFrame.TEMP_OFFSET, by3);
					cfs.add(cf);
				}
				
				for(CommunicationFrame cf : cfs)
					this.sendCommand(cf);
			} catch(Exception e){
				TextView debugLabel = (TextView)findViewById(R.id.debugLabel);
				StringWriter sw = new StringWriter();
				PrintWriter pw = new PrintWriter(sw);
				e.printStackTrace(pw);
				
				debugLabel.append("error\n");
				debugLabel.append(sw.toString());
			}
		}

	}

}