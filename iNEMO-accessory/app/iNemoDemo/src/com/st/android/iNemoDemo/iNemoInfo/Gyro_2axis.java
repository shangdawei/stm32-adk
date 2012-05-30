package com.st.android.iNemoDemo.iNemoInfo;

import com.st.android.iNemoDemo.CommunicationFrame;

public class Gyro_2axis extends Sensor{
	
	public Gyro_2axis(){
		super();
		this.parameters.put(CommunicationFrame.GYRO2_FULL_SCALE, new byte[1]);
		this.parameters.put(CommunicationFrame.GYRO2_OFFSET_X, new byte[2]);
		this.parameters.put(CommunicationFrame.GYRO2_OFFSET_Y, new byte[2]);
	}
}
