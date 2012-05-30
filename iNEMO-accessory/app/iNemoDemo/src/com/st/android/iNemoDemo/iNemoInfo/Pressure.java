package com.st.android.iNemoDemo.iNemoInfo;

import com.st.android.iNemoDemo.CommunicationFrame;

public class Pressure extends Sensor{
	
	public Pressure(){
		super();
		this.parameters.put(CommunicationFrame.PRESS_OUT_RATE, new byte[1]);
		this.parameters.put(CommunicationFrame.PRESS_OFFSET, new byte[2]);
	}
}
