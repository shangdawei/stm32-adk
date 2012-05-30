package com.st.android.iNemoDemo.iNemoInfo;

import java.util.HashMap;

public abstract class Sensor {

	private boolean enabled;
	protected HashMap<Byte, byte[]> parameters;
	
	public Sensor(){
		super();
		enabled = false;
		this.parameters = new HashMap<Byte, byte[]>();
	}

	public boolean isEnabled() {
		return enabled;
	}

	public void setEnabled(boolean enabled) {
		this.enabled = enabled;
	}

	public HashMap<Byte, byte[]> getParameters() {
		return parameters;
	}
	
	public byte[] getParamaterValue(byte parameter){
		if(!this.parameters.containsKey(parameter))
			return null;
		return this.parameters.get(parameter);
	}

	public boolean updateParameter(byte parameter, byte[] value){
		if(!this.parameters.containsKey(parameter))
			return false;
		if(value.length != this.parameters.get(parameter).length)
			return false;
		for(int i = 0; i < value.length; i++)
			this.parameters.get(parameter)[i] = value[i];
		return true;
	}
	
	public void resetParameters(){
		for(Byte b : this.parameters.keySet()){
			int len = this.parameters.get(b).length;
			for(int i = 0; i < len; i++)
				this.parameters.get(b)[i] = 0x00;
		}
	}
}
