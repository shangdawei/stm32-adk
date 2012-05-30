package com.st.android.iNemoDemo.iNemoInfo;

import java.util.HashMap;

import com.st.android.iNemoDemo.CommunicationFrame;

public class INemoInfo {

	private String UID;
	private String FW_v;
	private String HW_v;	
	private HashMap<Byte, Sensor> sensors;
	private HashMap<Byte, short[]> tmpAcquiredData;
	boolean calibratedData;
	boolean ahrsLib;
	byte acquisitionRate;
	
	public INemoInfo(){
		this.sensors = new HashMap<Byte, Sensor>();
		this.tmpAcquiredData = new HashMap<Byte, short[]>();
		this.sensors.put(CommunicationFrame.ACCELEROMETER, new Accelerometer());
		this.tmpAcquiredData.put(CommunicationFrame.ACCELEROMETER, new short[3]);
		this.sensors.put(CommunicationFrame.MAGNETOMETER, new Magnetometer());
		this.tmpAcquiredData.put(CommunicationFrame.MAGNETOMETER, new short[3]);
		this.sensors.put(CommunicationFrame.GYRO_2AXIS, new Gyro_2axis());
		this.tmpAcquiredData.put(CommunicationFrame.GYRO_2AXIS, new short[2]);
		this.sensors.put(CommunicationFrame.GYRO_1AXIS, new Gyro_1axis());
		this.tmpAcquiredData.put(CommunicationFrame.GYRO_1AXIS, new short[1]);
		this.sensors.put(CommunicationFrame.PRESSURE, new Pressure());
		this.tmpAcquiredData.put(CommunicationFrame.PRESSURE, new short[1]);
		this.sensors.put(CommunicationFrame.TEMPERATURE, new Temperature());
		this.tmpAcquiredData.put(CommunicationFrame.TEMPERATURE, new short[1]);
		this.reset();
	}
	
	public void reset(){
		this.UID = " ";
		this.FW_v = " ";
		this.HW_v = " ";
		this.calibratedData = true;
		this.ahrsLib = false;
		this.acquisitionRate = 0;
		for(Byte b : this.sensors.keySet())
			this.sensors.get(b).resetParameters();
	}

	public String getUID() {
		return UID;
	}

	public void setUID(String uID) {
		UID = uID;
	}

	public String getFW_v() {
		return FW_v;
	}

	public void setFW_v(String fW_v) {
		FW_v = fW_v;
	}

	public String getHW_v() {
		return HW_v;
	}

	public void setHW_v(String hW_v) {
		HW_v = hW_v;
	}
	
	public boolean isCalibratedData() {
		return calibratedData;
	}

	public void setCalibratedData(boolean calibratedData) {
		this.calibratedData = calibratedData;
	}

	public byte getAcquisitionRate() {
		return acquisitionRate;
	}

	public void setAcquisitionRate(byte acquisitionRate) {
		this.acquisitionRate = acquisitionRate;
	}
	
	public boolean isAhrsLib() {
		return ahrsLib;
	}

	public void setAhrsLib(boolean ahrsLib) {
		this.ahrsLib = ahrsLib;
	}

	public HashMap<Byte, short[]> getTmpAcquiredData() {
		return tmpAcquiredData;
	}

	public HashMap<Byte, Sensor> getSensors() {
		return sensors;
	}

	public boolean updateSensorParameter(byte sensor, byte parameter, byte[] value){
		if(!this.sensors.containsKey(sensor))
			return false;
		return this.sensors.get(sensor).updateParameter(parameter, value);
	}
	
	public byte[] getSensorParameterValue(byte sensor, byte parameter){
		if(!this.sensors.containsKey(sensor))
			return null;
		return this.sensors.get(sensor).getParamaterValue(parameter);
	}
	
	public void enableSensor(byte sensor, boolean enabled){
		if(this.sensors.containsKey(sensor))
			this.sensors.get(sensor).setEnabled(enabled);
	}
	
	public boolean isSensorEnabled(byte sensor){
		if(this.sensors.containsKey(sensor))
			return this.sensors.get(sensor).isEnabled();
		return false;
	}
	
	public void updateAcquiredData(byte sensor, short[] values){
		if(this.tmpAcquiredData.containsKey(sensor) && values.length == this.tmpAcquiredData.get(sensor).length){
			for(int i = 0; i < values.length; i++)
				this.tmpAcquiredData.get(sensor)[i] = values[i];
		}			
	}
	
	public String toString(){
		String s = "UID: " + this.UID + "\n";
		s += "FW: " + this.FW_v + "\n";
		s += "HW: " + this.HW_v + "\n";
		if(this.calibratedData)
			s += "Data: Calibrated\n";
		else
			s += "Data: Raw\n";
		if(this.ahrsLib)
			s += "AHRS Library: enabled\n";
		else
			s += "AHRS Library: disabled\n";
		s += "Acquisition Rate: ";
		if(this.acquisitionRate == CommunicationFrame.LOW_FREQUENCY)
			s += "1Hz\n";
		else if(this.acquisitionRate == CommunicationFrame.MEDIUM_FREQUENCY_1)
			s += "10Hz\n";
		else if(this.acquisitionRate == CommunicationFrame.MEDIUM_FREQUENCY_2)
			s += "25Hz\n";
		else if(this.acquisitionRate == CommunicationFrame.MEDIUM_FREQUENCY_3)
			s += "30Hz\n";
		else if(this.acquisitionRate == CommunicationFrame.HIGH_FREQUENCY_1)
			s += "50Hz\n";
		else if(this.acquisitionRate == CommunicationFrame.HIGH_FREQUENCY_2)
			s += "100Hz\n";
		else if(this.acquisitionRate == CommunicationFrame.HIGH_FREQUENCY_3)
			s += "400Hz\n";
		for(Byte b : this.sensors.keySet()){
			if(this.sensors.get(b).isEnabled()){
				s += CommunicationFrame.getSensorName(b) + ":\n";
				for(Byte b2 : this.sensors.get(b).parameters.keySet()){
					s += "\t- " + CommunicationFrame.getParameterName(b, b2) + ": " 
							+ CommunicationFrame.getParameterValueName(b, b2, this.sensors.get(b).parameters.get(b2))
							+ "\n";
				}
			}
		}
		return s;
	}
	
	public String printAcquisitionData(){
		String s = "";
		for(Byte b : this.sensors.keySet()){
			if(this.sensors.get(b).isEnabled()){
				s += CommunicationFrame.getSensorName(b) + ":";
				if(b == CommunicationFrame.ACCELEROMETER){
					s+= "\n";
					s+= "\tx: " + this.tmpAcquiredData.get(b)[0] + " mg\n";
					s+= "\ty: " + this.tmpAcquiredData.get(b)[1] + " mg\n";
					s+= "\tz: " + this.tmpAcquiredData.get(b)[2] + " mg\n";
				}
				else if(b == CommunicationFrame.MAGNETOMETER){
					s+= "\n";
					s+= "\tx: " + this.tmpAcquiredData.get(b)[0] + " mG\n";
					s+= "\ty: " + this.tmpAcquiredData.get(b)[1] + " mG\n";
					s+= "\tz: " + this.tmpAcquiredData.get(b)[2] + " mG\n";
				}
				else if(b == CommunicationFrame.GYRO_2AXIS){
					s+= "\n";
					s+= "\tx: " + this.tmpAcquiredData.get(b)[0] + " dps\n";
					s+= "\ty: " + this.tmpAcquiredData.get(b)[1] + " dps\n";
				}
				else if(b == CommunicationFrame.GYRO_1AXIS){
					s+= "\n";
					s+= "\tz: " + this.tmpAcquiredData.get(b)[0] + " dps\n";
				}
				else if(b == CommunicationFrame.PRESSURE){
					s+= "\t" + (float)(this.tmpAcquiredData.get(b)[0]/10.0f) + " mbar\n";
				}
				else if(b == CommunicationFrame.TEMPERATURE){
					s+= "\t" + (float)(this.tmpAcquiredData.get(b)[0]/10.0f) + " °C\n";
				}
			}
		}
		return s;
	}
}
