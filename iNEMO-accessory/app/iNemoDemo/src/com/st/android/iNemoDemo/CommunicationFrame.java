package com.st.android.iNemoDemo;

import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

import com.st.android.iNemoDemo.iNemoInfo.INemoInfo;

public class CommunicationFrame {
	
	static final String TAG = "CommunicationFrame";
	
	private static final int FRAME_SIZE = 64;
	
	/* Frame Control */
	private static final byte CTRL_type 	= 0x00; 		/* control frame */
	private static final byte DATA_type     = 0x40; 		/* data frame */
	private static final byte ACK_type 		= (byte)0x80; 	/* Ack frame */
	private static final byte NACK_type 	= (byte)0xC0; 	/* NACK frame */

	private static final byte ACK_req		= 0x20; 		/* Ack required */
	private static final byte ACK_NOTreq	= 0x00; 		/* NACK required */

	private static final byte Last_Frag 	= 0x00; 		/*  Last Fragment */
	private static final byte More_Frag		= 0x10; 		/* More Fragment */

	private static final byte Version_1		= 0x00; 		/* Frame Version */

	private static final byte QoS_Normal	= 0x00; 		/* Data no Ack Last Fragment */
	private static final byte Qos_Medium	= 0x01; 		/* Data with Ack More Fragment */
	private static final byte QoS_High		= 0x02; 		/* Ack no payload */

	/* Error codes */

	private static final byte CmdUnsupported   	= 0x01;
	private static final byte ValueOutOfRange  	= 0x02;
	private static final byte NotExecutable    	= 0x03;
	private static final byte WrongSyntax      	= 0x04;
	private static final byte iNEMONotConnected	= 0x05;

	/* Message IDs - Communication Control Frames */

	private static final byte iNEMO_Connect			= 0x00;
	private static final byte iNEMO_Disconnect		= 0x01;
	private static final byte iNEMO_Reset_Board		= 0x02;
	private static final byte iNEMO_Enter_DFU_Mode	= 0x03;
	private static final byte iNEMO_Trace			= 0x07;
	private static final byte iNEMO_Led				= 0x08;

	/* Message IDs - Board Info Frames */ 

	private static final byte iNEMO_Get_Device_Mode		= 0x10;
	private static final byte iNEMO_Get_MCU_ID			= 0x12;
	private static final byte iNEMO_Get_FW_Version		= 0x13;
	private static final byte iNEMO_Get_HW_Version		= 0x14;
	private static final byte iNEMO_Identify			= 0x15;
	private static final byte iNEMO_Get_AHRS_Library	= 0x17;
	private static final byte iNEMO_Get_Libraries		= 0x18;

	/* Message IDs - Sensor Setting Frames */

	private static final byte iNEMO_Set_Sensor_Parameter		= 0x20;
	private static final byte iNEMO_Get_Sensor_Parameter		= 0x21;
	private static final byte iNEMO_Restore_Default_Parameter	= 0x22;

	/* Message IDs - Acquisition Sensor Data Frames	*/

	private static final byte iNEMO_SetOutMode			= 0x50;
	private static final byte iNEMO_GetOutMode			= 0x51;
	private static final byte iNEMO_Start_Acquisition	= 0x52;
	private static final byte iNEMO_Stop_Acquisition	= 0x53;

	/* FREQUENCY ACQUISITION VALUES */

	public static final byte LOW_FREQUENCY        	= 0x00;    /* 1 HZ frequency acquisition */
	public static final byte MEDIUM_FREQUENCY_1		= 0x08;    /* 10 HZ frequency acquisition */
	public static final byte MEDIUM_FREQUENCY_2    	= 0x10;    /* 25 HZ frequency acquisition */
	public static final byte MEDIUM_FREQUENCY_3    	= 0x20;    /* 30 HZ frequency acquisition */
	public static final byte HIGH_FREQUENCY_1      	= 0x18;    /* 50 HZ frequency acquisition */
	public static final byte HIGH_FREQUENCY_2      	= 0x28;    /* 100 HZ frequency acquisition */
	public static final byte HIGH_FREQUENCY_3     	= 0x30;    /* 400 HZ frequency acquisition */
	
	/* Sensor Types */
	
	public static final byte ACCELEROMETER	= 0x00;
	public static final byte MAGNETOMETER   = 0x01;
	public static final byte GYRO_2AXIS  	= 0x02;
	public static final byte GYRO_1AXIS    	= 0x03;
	public static final byte PRESSURE     	= 0x04;
	public static final byte TEMPERATURE   	= 0x05;
	
	/* Accelerometer Parameters */
	
	public static final byte ACC_OUT_RATE	= 0x00;
	public static final byte ACC_FULL_SCALE	= 0x01;
	public static final byte ACC_HPF		= 0x02;
	public static final byte ACC_OFFSET_X	= 0x03;
	public static final byte ACC_OFFSET_Y	= 0x04;
	public static final byte ACC_OFFSET_Z	= 0x05;
	
	/* Accelerometer output data rate values */
	
	public static final byte ACC_OUT_RATE_50HZ		= 0x00;
	public static final byte ACC_OUT_RATE_100HZ		= 0x01;
	public static final byte ACC_OUT_RATE_400HZ		= 0x02;
	public static final byte ACC_OUT_RATE_1000HZ	= 0x03;
	
	/* Accelerometer full scale values */
	
	public static final byte ACC_FULL_SCALE_2G		= 0x00;
	public static final byte ACC_FULL_SCALE_4G		= 0x01;
	public static final byte ACC_FULL_SCALE_8G		= 0x03;
	
	/* Accelerometer high-pass filter */
	
	// TODO
	
	/* Magnetometer Parameters */
	
	public static final byte MAG_OUT_RATE	= 0x00;
	public static final byte MAG_FULL_SCALE	= 0x01;
	public static final byte MAG_OP_MODE	= 0x02;
	public static final byte MAG_OFFSET_X	= 0x03;
	public static final byte MAG_OFFSET_Y	= 0x04;
	public static final byte MAG_OFFSET_Z	= 0x05;
	
	/* Magnetometer output data rate values */
	
	public static final byte MAG_OUT_RATE_0_75HZ	= 0x00;
	public static final byte MAG_OUT_RATE_1_5HZ		= 0x01;
	public static final byte MAG_OUT_RATE_3HZ		= 0x02;
	public static final byte MAG_OUT_RATE_7_5HZ		= 0x03;
	public static final byte MAG_OUT_RATE_15HZ		= 0x04;
	public static final byte MAG_OUT_RATE_30HZ		= 0x05;
	public static final byte MAG_OUT_RATE_75HZ		= 0x06;
	
	/* Magnetometer full scale values */
	
	public static final byte MAG_FULL_SCALE_1_3G	= 0x00;
	public static final byte MAG_FULL_SCALE_1_9G	= 0x01;
	public static final byte MAG_FULL_SCALE_2_5G	= 0x03;
	public static final byte MAG_FULL_SCALE_4_0G	= 0x04;
	public static final byte MAG_FULL_SCALE_4_7G	= 0x05;
	public static final byte MAG_FULL_SCALE_5_6G	= 0x06;
	public static final byte MAG_FULL_SCALE_8_1G	= 0x07;
	
	/* Magnetometer operating modes values */
	
	public static final byte MAG_OP_MODE_NORMAL		= 0x00;
	public static final byte MAG_OP_MODE_POS_BIAS	= 0x01;
	public static final byte MAG_OP_MODE_NEG_BIAS	= 0x02;
	public static final byte MAG_OP_MODE_FORBIDDEN	= 0x03;
	
	/* 2-Axis Gyroscope Parameters */
	
	public static final byte GYRO2_FULL_SCALE	= 0x00;
	public static final byte GYRO2_OFFSET_X		= 0x01;
	public static final byte GYRO2_OFFSET_Y		= 0x02;
	
	/* 2-Axis Gyroscope full scale values */
	
	public static final byte GYRO2_FULL_SCALE_300	= 0x04;
	public static final byte GYRO2_FULL_SCALE_1200	= 0x08;
	
	/* 1-Axis Gyroscope Parameters */
	
	public static final byte GYRO1_FULL_SCALE	= 0x00;
	public static final byte GYRO1_OFFSET_Z		= 0x01;
	
	/* 1-Axis Gyroscope full scale values */
	
	public static final byte GYRO1_FULL_SCALE_300	= 0x04;
	
	/* Pressure Parameters */
	
	public static final byte PRESS_OUT_RATE	= 0x00;
	public static final byte PRESS_OFFSET	= 0x01;
	
	/* Pressure output data rate values */
	
	public static final byte PRESS_OUT_RATE_7HZ		= 0x01;
	public static final byte PRESS_OUT_RATE_12_5HZ	= 0x03;
	
	/* Temperature Parameters */
	
	public static final byte TEMP_OFFSET	= 0x00;
	
	/* Frame message */
	private byte[] msg;
	
	public CommunicationFrame(){
		//this.msg = new byte[FRAME_SIZE + 1];
	}
	
	public byte[] getMsg(){
		return this.msg;
	}
	
	public void setMsg(byte[] message){
		int len = message.length < FRAME_SIZE ? message.length : FRAME_SIZE;
		this.msg = new byte[len];
		for(int i = 0; i < len; i++){
			this.msg[i] = message[i];
		}
	}
	
	public void setConnectFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_Connect;
	}
	
	public void setDisconnectFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_Disconnect;
	}
	
	public void setGetMCUIDFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_Get_MCU_ID;
	}
	
	public void setGetFWVersionFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_Get_FW_Version;
	}
	
	public void setGetHWVersionFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_Get_HW_Version;
	}
	
	public void setSetSensorParameterFrame(byte sensor, byte parameter, byte[] value){		
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;		
		int v_len = value.length;
		
		this.msg = new byte[5 + v_len];
		this.msg[0] = frame_ctrl;
		this.msg[1] = (byte) (0x03 + v_len);
		this.msg[2] = iNEMO_Set_Sensor_Parameter;
		this.msg[3] = sensor;
		this.msg[4] = parameter;
		this.msg[5] = value[0];
		if(v_len > 1)
			this.msg[6] = value[1];
	}
	
	public void setGetSensorParameterFrame(byte sensor, byte parameter){		
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;		
		
		this.msg = new byte[5];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x03;
		this.msg[2] = iNEMO_Get_Sensor_Parameter;
		this.msg[3] = sensor;
		this.msg[4] = parameter;
	}
	
	public void setRestoreDefaultParameterFrame(byte sensor, byte parameter){		
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;		
		
		this.msg = new byte[5];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x03;
		this.msg[2] = iNEMO_Restore_Default_Parameter;
		this.msg[3] = sensor;
		this.msg[4] = parameter;
	}
	
	public void setSetOutputModeFrame(boolean tempEnabled, boolean pressEnabled, boolean magEnabled, boolean gyroEnabled,
			boolean accEnabled, boolean calibratedData, boolean ahrsEnabled, byte acquisitionRate){		
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;		
		byte out1 = 0x00, out2 = 0x00;
		
		if(tempEnabled)
			out1 |= 0x01;
		if(pressEnabled)
			out1 |= 0x02;
		if(magEnabled)
			out1 |= 0x04;
		if(gyroEnabled)
			out1 |= 0x08;
		if(accEnabled)
			out1 |= 0x10;
		if(!calibratedData)
			out1 |= 0x20;
		if(ahrsEnabled)
			out1 |= 0x80;
		
		out2 |= acquisitionRate;
		
		this.msg = new byte[7];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x05;
		this.msg[2] = iNEMO_SetOutMode;
		this.msg[3] = out1;
		this.msg[4] = out2;
		this.msg[5] = 0x00;
		this.msg[6] = 0x00;
	}
	
	public void setGetOutputModeFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_GetOutMode;
	}
	
	public void setStartAcquisitionFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_Start_Acquisition;
	}
	
	public void setStopAcquisitionFrame(){
		byte frame_ctrl = CTRL_type | ACK_req | Last_Frag | Version_1 | QoS_Normal;
		
		this.msg = new byte[3];
		this.msg[0] = frame_ctrl;
		this.msg[1] = 0x01;
		this.msg[2] = iNEMO_Stop_Acquisition;
	}
	
	public boolean isConnectACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x01)
			return false;
		if(this.msg[2] != iNEMO_Connect)
			return false;
		return true;
	}
	
	public boolean isConnectNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Connect)
			return false;
		return true;
	}
	
	public boolean isDisconnectACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x01)
			return false;
		if(this.msg[2] != iNEMO_Disconnect)
			return false;
		return true;
	}
	
	public boolean isDisconnectNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Disconnect)
			return false;
		return true;
	}
	
	public boolean isGetMCUIDACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x0d)
			return false;
		if(this.msg[2] != iNEMO_Get_MCU_ID)
			return false;
		return true;
	}
	
	public boolean isGetMCUIDNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Get_MCU_ID)
			return false;
		return true;
	}
	
	public boolean isGetFWVersionACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] < 0x02)
			return false;
		if(this.msg[2] != iNEMO_Get_FW_Version)
			return false;
		return true;
	}
	
	public boolean isGetFWVersionNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Get_FW_Version)
			return false;
		return true;
	}
	
	public boolean isGetHWVersionACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] < 0x02)
			return false;
		if(this.msg[2] != iNEMO_Get_HW_Version)
			return false;
		return true;
	}
	
	public boolean isGetHWVersionNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Get_HW_Version)
			return false;
		return true;
	}
	
	public boolean isSetSensorParameterACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x01)
			return false;
		if(this.msg[2] != iNEMO_Set_Sensor_Parameter)
			return false;
		return true;
	}
	
	public boolean isSetSensorParameterNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Set_Sensor_Parameter)
			return false;
		return true;
	}
	
	public boolean isGetSensorParameterACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] < 0x02)
			return false;
		if(this.msg[2] != iNEMO_Get_Sensor_Parameter)
			return false;
		return true;
	}
	
	public boolean isGetSensorParameterNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Get_Sensor_Parameter)
			return false;
		return true;
	}
	
	public boolean isRestoreDefaultParameterACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] < 0x02)
			return false;
		if(this.msg[2] != iNEMO_Restore_Default_Parameter)
			return false;
		return true;
	}
	
	public boolean isRestoreDefaultParameterNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Restore_Default_Parameter)
			return false;
		return true;
	}
	
	public boolean isSetOutputModeACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x01)
			return false;
		if(this.msg[2] != iNEMO_SetOutMode)
			return false;
		return true;
	}
	
	public boolean isSetOutputModeNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_SetOutMode)
			return false;
		return true;
	}
	
	public boolean isGetOutputModeACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x05)
			return false;
		if(this.msg[2] != iNEMO_GetOutMode)
			return false;
		return true;
	}
	
	public boolean isGetOutputModeNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_GetOutMode)
			return false;
		return true;
	}
	
	public boolean isStartAcquisitonACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x01)
			return false;
		if(this.msg[2] != iNEMO_Start_Acquisition)
			return false;
		return true;
	}
	
	public boolean isStartAcquisitonNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Start_Acquisition)
			return false;
		return true;
	}
	
	public boolean isStopAcquisitonACK(){
		if(this.msg[0] != (ACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x01)
			return false;
		if(this.msg[2] != iNEMO_Stop_Acquisition)
			return false;
		return true;
	}
	
	public boolean isStopAcquisitonNACK(){
		if(this.msg[0] != (NACK_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] != 0x02)
			return false;
		if(this.msg[2] != iNEMO_Stop_Acquisition)
			return false;
		return true;
	}
	
	public boolean isAcquisitionData(){
		if(this.msg[0] != (DATA_type | ACK_NOTreq | Last_Frag | Version_1 | QoS_Normal))
			return false;
		if(this.msg[1] < 0x03)
			return false;
		if(this.msg[2] != iNEMO_Start_Acquisition)
			return false;
		return true;
	}
	
	public static byte[] convertShortInBytes(short value){
		ByteBuffer buffer = ByteBuffer.allocate(2);
		buffer.putShort(value);
		buffer.flip();
		return buffer.array();
	}
	
	public static short convertBytesInShort(byte[] values){
		ByteBuffer buffer = ByteBuffer.wrap(values);
		ShortBuffer shorts = buffer.asShortBuffer( );
		return shorts.get(0);
	}
	
	public String getErrorCode(){
		if(this.msg[1] == 0x02){
			if(this.msg[3] == CmdUnsupported)
				return "Command Not Supported";
			else if(this.msg[3] == ValueOutOfRange)
				return "Value Out Of Range";
			else if(this.msg[3] == NotExecutable)
				return "Not Executable";
			else if(this.msg[3] == WrongSyntax)
				return "Wrong Syntax";
			else if(this.msg[3] == iNEMONotConnected)
				return "iNEMO Not Connected";
		}
		return "Error code undefined";
	}	
	
	public String getStringPayload(){
		String payload = "";
		int len = this.msg[1] - 1;
		for(int i = 0; i < len ; i++){
			payload +=  (char)this.msg[3 + i];
		}
		return payload;	
	}
	
	
	public static String getSensorName(byte sensor){
		String s;
		if(sensor == ACCELEROMETER)
			s = "Accelerometer";
		else if(sensor == MAGNETOMETER)
			s = "Magnetometer";
		else if(sensor == GYRO_2AXIS)
			s = "Gyro 2-axis";
		else if(sensor == GYRO_1AXIS)
			s = "Gyro 1-axis";
		else if(sensor == PRESSURE)
			s = "Pressure";
		else if(sensor == TEMPERATURE)
			s = "Temperature";
		else
			s = "";
		return s;
	}
	
	public static String getParameterName(byte sensor, byte parameter){
		String s;
		if(sensor == ACCELEROMETER){
			if(parameter == ACC_OUT_RATE)
				s = "Output Data Rate";
			else if(parameter == ACC_FULL_SCALE)
				s = "Full Scale";
			else if(parameter == ACC_HPF)
				s = "HP filter";
			else if(parameter == ACC_OFFSET_X)
				s = "Offset x";
			else if(parameter == ACC_OFFSET_Y)
				s = "Offset y";
			else if(parameter == ACC_OFFSET_Z)
				s = "Offset z";
			else
				s = "";
		}
		else if(sensor == MAGNETOMETER){
			if(parameter == MAG_OUT_RATE)
				s = "Output Data Rate";
			else if(parameter == MAG_FULL_SCALE)
				s = "Full Scale";
			else if(parameter == MAG_OP_MODE)
				s = "Operating Mode";
			else if(parameter == MAG_OFFSET_X)
				s = "Offset x";
			else if(parameter == MAG_OFFSET_Y)
				s = "Offset y";
			else if(parameter == MAG_OFFSET_Z)
				s = "Offset z";
			else
				s = "";
		}
		else if(sensor == GYRO_2AXIS){
			if(parameter == GYRO2_FULL_SCALE)
				s = "Full Scale";
			else if(parameter == GYRO2_OFFSET_X)
				s = "Offset x";
			else if(parameter == GYRO2_OFFSET_Y)
				s = "Offset y";
			else
				s = "";
		}
		else if(sensor == GYRO_1AXIS){
			if(parameter == GYRO1_FULL_SCALE)
				s = "Full Scale";
			else if(parameter == GYRO1_OFFSET_Z)
				s = "Offset z";
			else
				s = "";
		}
		else if(sensor == PRESSURE){
			if(parameter == PRESS_OUT_RATE)
				s = "Output Data Rate";
			else if(parameter == PRESS_OFFSET)
				s = "Offset";
			else
				s = "";
		}
		else if(sensor == TEMPERATURE){
			if(parameter == TEMP_OFFSET)
				s = "Offset";
			else
				s = "";
		}
		else
			s = "";
		return s;
	}
	
	public static String getParameterValueName(byte sensor, byte parameter, byte[] value){
		String s;
		if(sensor == ACCELEROMETER){
			if(parameter == ACC_OUT_RATE){
				if(value[0] == ACC_OUT_RATE_50HZ)
					s = "50Hz";
				else if(value[0] == ACC_OUT_RATE_100HZ)
					s = "100Hz";
				else if(value[0] == ACC_OUT_RATE_400HZ)
					s = "400Hz";
				else if(value[0] == ACC_OUT_RATE_1000HZ)
					s = "1000Hz";
				else
					s = "";
			}
			else if(parameter == ACC_FULL_SCALE){
				if(value[0] == ACC_FULL_SCALE_2G)
					s = "2g";
				else if(value[0] == ACC_FULL_SCALE_4G)
					s = "4g";
				else if(value[0] == ACC_FULL_SCALE_8G)
					s = "8g";
				else
					s = "";
			}
			else if(parameter == ACC_HPF){
				// TODO
				s = "";
			}
			else if(parameter == ACC_OFFSET_X){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else if(parameter == ACC_OFFSET_Y){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else if(parameter == ACC_OFFSET_Z){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else
				s = "";
		}
		else if(sensor == MAGNETOMETER){
			if(parameter == MAG_OUT_RATE){
				if(value[0] == MAG_OUT_RATE_0_75HZ)
					s = "0.75Hz";
				else if(value[0] == MAG_OUT_RATE_1_5HZ)
					s = "1.5Hz";
				else if(value[0] == MAG_OUT_RATE_3HZ)
					s = "3Hz";
				else if(value[0] == MAG_OUT_RATE_7_5HZ)
					s = "7.5Hz";
				else if(value[0] == MAG_OUT_RATE_15HZ)
					s = "15Hz";
				else if(value[0] == MAG_OUT_RATE_30HZ)
					s = "30Hz";
				else if(value[0] == MAG_OUT_RATE_75HZ)
					s = "75Hz";
				else
					s = "";
			}
			else if(parameter == MAG_FULL_SCALE){
				if(value[0] == MAG_FULL_SCALE_1_3G)
					s = "0.75Hz";
				else if(value[0] == MAG_FULL_SCALE_1_3G)
					s = "1.3g";
				else if(value[0] == MAG_FULL_SCALE_1_9G)
					s = "1.9g";
				else if(value[0] == MAG_FULL_SCALE_2_5G)
					s = "2.5g";
				else if(value[0] == MAG_FULL_SCALE_4_0G)
					s = "4.0g";
				else if(value[0] == MAG_FULL_SCALE_4_7G)
					s = "4.7g";
				else if(value[0] == MAG_FULL_SCALE_5_6G)
					s = "5.6g";
				else if(value[0] == MAG_FULL_SCALE_8_1G)
					s = "8.1g";
				else
					s = "";
			}
			else if(parameter == MAG_OP_MODE){
				if(value[0] == MAG_OP_MODE_NORMAL)
					s = "Normal";
				else if(value[0] == MAG_OP_MODE_POS_BIAS)
					s = "Positive Bias";
				else if(value[0] == MAG_OP_MODE_NEG_BIAS)
					s = "Negative Bias";
				else if(value[0] == MAG_OP_MODE_FORBIDDEN)
					s = "Forbidden";
				else
					s = "";
			}
			else if(parameter == MAG_OFFSET_X){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else if(parameter == MAG_OFFSET_Y){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else if(parameter == MAG_OFFSET_Z){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else
				s = "";
		}
		else if(sensor == GYRO_2AXIS){
			if(parameter == GYRO2_FULL_SCALE){
				if(value[0] == GYRO2_FULL_SCALE_300)
					s = "300dps";
				else if(value[0] == GYRO2_FULL_SCALE_1200)
					s = "1200dps";
				else
					s = "";
			}
			else if(parameter == GYRO2_OFFSET_X){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else if(parameter == GYRO2_OFFSET_Y){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else
				s = "";
		}
		else if(sensor == GYRO_1AXIS){
			if(parameter == GYRO1_FULL_SCALE){
				if(value[0] == GYRO1_FULL_SCALE_300)
					s = "300dps";
				else
					s = "";
			}
			else if(parameter == GYRO1_OFFSET_Z){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else
				s = "";
		}
		else if(sensor == PRESSURE){
			if(parameter == PRESS_OUT_RATE)
				s = "Output Data Rate";
			else if(parameter == PRESS_OFFSET){
				short v = convertBytesInShort(value);
				s = "" + v;
			}
			else
				s = "";
		}
		else if(sensor == TEMPERATURE){
			if(parameter == TEMP_OFFSET){
				short v = convertBytesInShort(value);
				s = "" + v;
			}		
			else
				s = "";
		}
		else
			s = "";
		return s;
	}
	
	public boolean updateParameter(INemoInfo info){
		if(info == null)
			return false;
		int len = (this.msg[1] - 3) <= 2 ? (this.msg[1] - 3) : 2;
		byte[] value = new byte[len];
		for(int i = 0; i < len; i++)
			value[i] = this.msg[5 + i];
		return info.updateSensorParameter(this.msg[3], this.msg[4], value);
	}
	
	public boolean updateOutputMode(INemoInfo info, SharedPreferences sharedPrefs){
		if(info == null || sharedPrefs == null)
			return false;
		Editor e = sharedPrefs.edit();
		if((this.msg[3] & 0x01) == 0x01){
			info.enableSensor(TEMPERATURE, true);
			e.putBoolean("enable_temp", true);
		} else{
			info.enableSensor(TEMPERATURE, false);
			e.putBoolean("enable_temp", false);
		}
		if((this.msg[3] & 0x02) == 0x02){
			info.enableSensor(PRESSURE, true);
			e.putBoolean("enable_press", true);
		} else{
			info.enableSensor(PRESSURE, false);
			e.putBoolean("enable_press", false);
		}
		if((this.msg[3] & 0x04) == 0x04){
			info.enableSensor(MAGNETOMETER, true);
			e.putBoolean("enable_mag", true);
		} else{
			info.enableSensor(MAGNETOMETER, false);
			e.putBoolean("enable_mag", false);
		}
		if((this.msg[3] & 0x08) == 0x08){
			info.enableSensor(GYRO_2AXIS, true);
			info.enableSensor(GYRO_1AXIS, true);
			e.putBoolean("enable_gyro", true);
		} else{
			info.enableSensor(GYRO_2AXIS, false);
			info.enableSensor(GYRO_1AXIS, false);
			e.putBoolean("enable_gyro", false);
		}
		if((this.msg[3] & 0x10) == 0x10){
			info.enableSensor(ACCELEROMETER, true);
			e.putBoolean("enable_acc", true);
		} else {
			info.enableSensor(ACCELEROMETER, false);
			e.putBoolean("enable_acc", false);
		}
		if((this.msg[3] & 0x20) == 0x20) {
			info.setCalibratedData(false);
			e.putBoolean("calibrated_data", false);
		} else {
			info.setCalibratedData(true);
			e.putBoolean("calibrated_data", true);
		}
		info.setAcquisitionRate(this.msg[4]);
		e.putString("acquisition_rate", Byte.toString(this.msg[4]));
		e.commit();
		return true;
	}
	
	public void updateAcquiredData(INemoInfo info){
		if(info == null)
			return;
		int count = 5;
		if(info.isSensorEnabled(ACCELEROMETER)){
			short[] values = new short[3];
			byte[] tmp = new byte[2];
			tmp[0] = this.msg[count];
			tmp[1] = this.msg[count + 1];
			values[0] = convertBytesInShort(tmp);
			tmp[0] = this.msg[count + 2];
			tmp[1] = this.msg[count + 3];
			values[1] = convertBytesInShort(tmp);
			tmp[0] = this.msg[count + 4];
			tmp[1] = this.msg[count + 5];
			values[2] = convertBytesInShort(tmp);
			info.updateAcquiredData(ACCELEROMETER, values);
			count += 6;
		}
		if(info.isSensorEnabled(GYRO_2AXIS)){
			short[] values = new short[2];
			byte[] tmp = new byte[2];
			tmp[0] = this.msg[count];
			tmp[1] = this.msg[count + 1];
			values[0] = convertBytesInShort(tmp);
			tmp[0] = this.msg[count + 2];
			tmp[1] = this.msg[count + 3];
			values[1] = convertBytesInShort(tmp);
			info.updateAcquiredData(GYRO_2AXIS, values);
			count += 4;
		}
		if(info.isSensorEnabled(GYRO_1AXIS)){
			short[] values = new short[1];
			byte[] tmp = new byte[2];
			tmp[0] = this.msg[count];
			tmp[1] = this.msg[count + 1];
			values[0] = convertBytesInShort(tmp);
			info.updateAcquiredData(GYRO_1AXIS, values);
			count += 2;
		}
		if(info.isSensorEnabled(MAGNETOMETER)){
			short[] values = new short[3];
			byte[] tmp = new byte[2];
			tmp[0] = this.msg[count];
			tmp[1] = this.msg[count + 1];
			values[0] = convertBytesInShort(tmp);
			tmp[0] = this.msg[count + 2];
			tmp[1] = this.msg[count + 3];
			values[1] = convertBytesInShort(tmp);
			tmp[0] = this.msg[count + 4];
			tmp[1] = this.msg[count + 5];
			values[2] = convertBytesInShort(tmp);
			info.updateAcquiredData(MAGNETOMETER, values);
			count += 6;
		}
		if(info.isSensorEnabled(PRESSURE)){
			short[] values = new short[1];
			byte[] tmp = new byte[2];
			tmp[0] = this.msg[count];
			tmp[1] = this.msg[count + 1];
			values[0] = convertBytesInShort(tmp);
			info.updateAcquiredData(PRESSURE, values);
			count += 2;
		}
		if(info.isSensorEnabled(TEMPERATURE)){
			short[] values = new short[1];
			byte[] tmp = new byte[2];
			tmp[0] = this.msg[count];
			tmp[1] = this.msg[count + 1];
			values[0] = convertBytesInShort(tmp);
			info.updateAcquiredData(TEMPERATURE, values);
			count += 2;
		}
	}
}
