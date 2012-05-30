package com.st.android.iNemoDemo.iNemoInfo;

import com.st.android.iNemoDemo.CommunicationFrame;

public class Gyro_1axis extends Sensor{
	
	public Gyro_1axis(){
		super();
		this.parameters.put(CommunicationFrame.GYRO1_FULL_SCALE, new byte[1]);
		this.parameters.put(CommunicationFrame.GYRO1_OFFSET_Z, new byte[2]);
	}
}
