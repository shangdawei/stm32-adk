package com.st.android.iNemoDemo.iNemoInfo;

import com.st.android.iNemoDemo.CommunicationFrame;

public class Accelerometer extends Sensor{
	
	public Accelerometer(){
		super();
		this.parameters.put(CommunicationFrame.ACC_OUT_RATE, new byte[1]);
		this.parameters.put(CommunicationFrame.ACC_FULL_SCALE, new byte[1]);
		this.parameters.put(CommunicationFrame.ACC_HPF, new byte[2]);
		this.parameters.put(CommunicationFrame.ACC_OFFSET_X, new byte[2]);
		this.parameters.put(CommunicationFrame.ACC_OFFSET_Y, new byte[2]);
		this.parameters.put(CommunicationFrame.ACC_OFFSET_Z, new byte[2]);
	}
	
}
