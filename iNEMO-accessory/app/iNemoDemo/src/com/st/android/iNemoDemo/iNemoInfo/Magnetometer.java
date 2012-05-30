package com.st.android.iNemoDemo.iNemoInfo;

import com.st.android.iNemoDemo.CommunicationFrame;

public class Magnetometer extends Sensor{
	
	public Magnetometer(){
		super();
		this.parameters.put(CommunicationFrame.MAG_OUT_RATE, new byte[1]);
		this.parameters.put(CommunicationFrame.MAG_FULL_SCALE, new byte[1]);
		this.parameters.put(CommunicationFrame.MAG_OP_MODE, new byte[1]);
		this.parameters.put(CommunicationFrame.MAG_OFFSET_X, new byte[2]);
		this.parameters.put(CommunicationFrame.MAG_OFFSET_Y, new byte[2]);
		this.parameters.put(CommunicationFrame.MAG_OFFSET_Z, new byte[2]);
	}
}
