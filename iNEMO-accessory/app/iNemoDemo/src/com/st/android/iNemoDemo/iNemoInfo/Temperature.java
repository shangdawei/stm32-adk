package com.st.android.iNemoDemo.iNemoInfo;

import com.st.android.iNemoDemo.CommunicationFrame;

public class Temperature extends Sensor{
	
	public Temperature(){
		super();
		this.parameters.put(CommunicationFrame.TEMP_OFFSET, new byte[2]);
	}
	
}
