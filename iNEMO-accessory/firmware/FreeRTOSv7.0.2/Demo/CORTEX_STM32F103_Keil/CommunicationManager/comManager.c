
#include "comManager.h"
#include "string.h"												
#include "utils.h"
#include "iNEMO_util.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_nvic.h"
#include <AndroidAccessory.h>


/**
 *  Specifies the incremental counter for the data frames sent to the device.
 */
static u16 s_iC = 0;

/**
 * Number of sample to be acquired
 */
static u16 s_nLengthSample = 0;



/**
*  Specifies which output data are enabled
*  Details: 0 Disable/1 Enable order bit :AHRS | RFU 0 | Cal/Raw | Accelerometer | Gyro | Magnetometer | Pressure | Temperature
*     * Cal/Raw   0-> Calibrated data 
*                 1-> Raw data [lsb]
*  0x1F --> all sensors enabled
*  0x11 --> only temperature and accelerometer sensors enabled
*/
static u8 s_uOutSelect = 0x11;


/**
*  Specifies comunicatin frequency and output channel 
*  Details :RFU 0 | RFU 0 | FQ2 | FQ1 | FQ0 | OT2 | OT1 | OT0
*      FQ : 000 (1Hz); 001 (10Hz); 010 (25Hz); 011 (50Hz) 
*      OT : 000 (USB output) 
*/

static u8 s_uOutMode=0;

/**
*  Specifies if the iNEMO is connected
*/

static bool s_bConnectState=FALSE;


/**
 *  Sensor sampling frequency
 */

static u8 s_uTimerFrequence=0x03;


/**
 * Specifies if the sensor data have to be filtered using AHRS.
 */
static bool s_bAhrsEnabled = FALSE;

/**
 * Specifies if the raw sensor data sending is enabled.
 */
static bool s_bRawDataEnabled = FALSE;


/**
 * Process a command coming from the device (through ADK connection). All supported command are: 
 * iNEMO_Connect - this must be the first command to send to the board. Others command are
 * not processed before this one.
 * iNEMO_Disconnect 
 * iNEMO_Reset_Board 
 * iNEMO_Enter_DFU_Mode 
 * iNEMO_Led_Control 
 * iNEMO_Get_Device_Mode 
 * iNEMO_Get_MCU_ID 
 * iNEMO_Get_FW_Version 
 * iNEMO_Get_HW_Version 
 * iNEMO_Get_Identify 
 * iNEMO_Get_AHRS_Library 
 * iNEMO_Get_Libraries 
 * iNEMO_SetOutMode 
 * iNEMO_GetOutMode  
 * iNEMO_Set_Sensor_Parameter
 * iNEMO_Get_Sensor_Parameter
 * iNEMO_Restore_Default_Parameter
 * iNEMO_Start_Acquisition - start the the transmission of sensor data according to the
 * parameters specified in frame payload.
 * iNEMO_Stop_Acquisition - stop the transmission of sensor data.
 * Invalid frame are  dropped.
 *
 * Parameters:
 * nFrameSize specifies the size of the received frame.
 * buffer_rx is the frame to be processed.
 * pdata contains the sensor parameters settings.
 */
void ParseCommandFrame(u32 nFrameSize, u8* buffer_rx, iNEMO_DATA * pdata) {

  /** Check if the iNEMO is connect, if it is disconnected the only accepted command is iNEMO_Connect */  
  if(s_bConnectState == FALSE )
  {
    /* if the command is iNEMO_Connect Parse the frame */ 
    if(buffer_rx[2]== iNEMO_Connect)
    {
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1 && nFrameSize== buffer_rx[1]+2)
      {  
        /* iNEMO_Connet_Response --> Change iNEMO Stauts to Connceted and Send Ack*/
        s_bConnectState=TRUE;
        Send_Ack(iNEMO_Connect,1,0x00);
      }
      else
      {
        /* wrong  syntax */
        Send_Nack(iNEMO_Connect,WrongSyntax);
      }
    }
    else  /* the command is not iNEMO_Connect */
    {
      Send_Nack((char)buffer_rx[2], iNEMONotConnected);
    }
  }
  else  /* if iNEMO is connected the other command can be executed   */
  {
    /* Switch the command */
    switch (buffer_rx[2]){
      
      /* Connection Request*/  
    case iNEMO_Connect:  
      /* syntactic check :
      Frame Control -> Control frame; Ach required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1 (No payload)
      nFrameSize --> check if the payload lenght is correct          
      */
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2)
      {  
        /* iNEMO_Connet_Response --> Change iNEMO Stauts to Connceted and Send Ack*/
        s_bConnectState=TRUE;
        Send_Ack(iNEMO_Connect,1,0x00);
      }
      else
      {
        Send_Nack(iNEMO_Connect,WrongSyntax);
      }
      break;
      
      /*  Disconnect Request*/   
    case iNEMO_Disconnect:
      /* syntactic check :
      Frame Control -> Control frame; Ach required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1 (No payload)
      nFrameSize --> check il the payload lenght is correct          
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2)
      {
        /* iNEMO_Connet_Response --> Change iNEMO Stauts to Disconnceted and Send Ack */
        s_bConnectState=FALSE;
        Send_Ack(iNEMO_Disconnect,1,0x00);
      }
      else
        Send_Nack(iNEMO_Disconnect,WrongSyntax);
      break;
      /* iNEMO_Get_MCU_ID Request */   
    case iNEMO_Get_MCU_ID:
      /* syntactic check :
      Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1 
      nFrameSize --> check il the payload lenght is correct          
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2 )  
      {
        unsigned char mcu_id[12];
		int i = 0;
        for (i=0; i<12; ++i)
          mcu_id[i] = MCU_ID[12-(i+1)];                        
        /* send ack with payload= STM32 Unique indetifier */            
        Send_Ack(iNEMO_Get_MCU_ID, 13, mcu_id);
      }
      else
        Send_Nack(iNEMO_Get_MCU_ID,WrongSyntax);
      break;  
      
      /*  iNEMO Get FW Version Request */   
    case iNEMO_Get_FW_Version:
      /* syntactic check :
      Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1 
      nFrameSize --> check il the payload lenght is correct          
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2 )  
      {
        /* send ack with payload= string FW version*/            
        Send_Ack(iNEMO_Get_FW_Version, (SIZE_FWversion + 1), iNEMO_FIRMWARE_VERSION);
      }
      else
        Send_Nack(iNEMO_Get_FW_Version,WrongSyntax);
      break;                       
      
      /*  iNEMO Get HW Version Request :  */   
    case iNEMO_Get_HW_Version:
      /* syntactic check :
      Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1 
      nFrameSize --> check il the payload lenght is correct          
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2 )  
      {
        /* send ack with payload= string HW version*/            
        Send_Ack(iNEMO_Get_HW_Version, (SIZE_HWversion + 1), iNEMO_HARDWARE_VERSION);
      }
      else
        Send_Nack(iNEMO_Get_HW_Version,WrongSyntax);
      break;      

/*------ Sensor Setting parameter---------- */
      
      /* Set parameter */
        case iNEMO_Set_Sensor_Parameter:
          /* syntactic check :
            Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
            Lenght -> variable 
            nFrameSize --> check il the payload lenght is correct
            buffer_rx[1] -->  Payload Lenght +1
            buffer_rx[3]  -->  Sensor Type
            buffer_rx[4]  --> Sensor Parameter
            buffer_rx[5] --> New value to set (if any)
          */
          if(buffer_rx[0]==CTRL_wACK_LF  && nFrameSize== buffer_rx[1]+2)
          {
            if(iNEMO_Set_Sensor(pdata, buffer_rx[3], buffer_rx[4], (buffer_rx[1] - 2), &buffer_rx[5])) 
            { 
               Send_Ack(iNEMO_Set_Sensor_Parameter, 1, 0x00);
            }
            else
            /* else send nack  */              
            Send_Nack(iNEMO_Set_Sensor_Parameter,NotExecutable);   
          }        
          else
           Send_Nack(iNEMO_Set_Sensor_Parameter,WrongSyntax);
          break;   
          
      /* Get parameter */
        case iNEMO_Get_Sensor_Parameter:
          /* syntactic check :
            Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
            Lenght -> variable 
            nFrameSize --> check il the payload lenght is correct
            buffer_rx[1] -->  Payload Lenght +1
            buffer_rx[3]  -->  Sensor Type
            buffer_rx[4]  --> Sensor Parameter
          */
          if(buffer_rx[0]==CTRL_wACK_LF  && nFrameSize== buffer_rx[1]+2)
          {
            u8 value[10];
            if(iNEMO_Get_Sensor_Param(pdata, buffer_rx[3], buffer_rx[4], value))
               Send_Ack(iNEMO_Get_Sensor_Parameter, value[0]+1, &value[1]);
            else
            /* else send nack  */              
            Send_Nack(iNEMO_Get_Sensor_Parameter,NotExecutable);   
          }        
          else
           Send_Nack(iNEMO_Get_Sensor_Parameter,WrongSyntax);
          break;  
          
      /* Restore Default Parameter */
        case iNEMO_Restore_Default_Parameter:          
          /* syntactic check :
            Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
            Lenght -> variable 
            nFrameSize --> check il the payload lenght is correct
            buffer_rx[1] -->  Payload Lenght +1
            buffer_rx[3]  -->  Sensor Type
            buffer_rx[4]  --> Sensor Parameter
          */
          if(buffer_rx[0]==CTRL_wACK_LF  && nFrameSize== buffer_rx[1]+2)
          {
            u8 value[10];
            if(iNEMO_Restore_DefaultParam(pdata, buffer_rx[3], buffer_rx[4], value))
               Send_Ack(iNEMO_Restore_Default_Parameter,  value[0]+1, &value[1]);
            else
            /* else send nack  */              
            Send_Nack(iNEMO_Restore_Default_Parameter,NotExecutable);   
          }        
          else
           Send_Nack(iNEMO_Restore_Default_Parameter,WrongSyntax);
          break;         
      
  /*-------Acquisition Command--------------- */          
      
      /* Configure iNEMO output settings*/
    case iNEMO_SetOutMode:
      /* syntactic check :
      Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
      Lenght -> 5 
      nFrameSize --> check il the payload lenght is correct
      buffer_rx[3]  --> Enable setting 1 Enable/0 Disable AHRS |RFU 0 | Cal/Raw | Acc | Gyro | Mag | Press | temp |
      buffer_rx[4]  --> Acquition Frequency + output type : 00 FQ2 FQ1 FQ0 OT2 OT1 OT0 
      buffer_rx[5]  --> Number of Samples (MSB)
      buffer_rx[6]  --> Number of Samples (LSB)
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==5  && nFrameSize== buffer_rx[1]+2 && ((buffer_rx[4] & 0xC0)==0x00)) 
      {    
       
#ifndef AHRS_MOD  /* The AHRS Library is not available */
        if(buffer_rx[3] & 0x80)
          Send_Nack(iNEMO_SetOutMode,NotExecutable);
        else
        {
          SetRawData(buffer_rx[3] & 0x20 ? TRUE : FALSE);
          s_uOutSelect=buffer_rx[3];  
          s_uTimerFrequence = (buffer_rx[4] & 0x38) >> 3;
          s_uOutMode =(buffer_rx[4] & 0x03);
          if((s_uTimerFrequence>6) || (s_uOutMode>0))
          {
            Send_Nack(iNEMO_SetOutMode,ValueOutOfRange);
          }
          else
          {
          /* set timer */
          Set_Timer(s_uTimerFrequence);
          /* set number of sample or continuos mode (s_nLengthSample=0) */
          s_nLengthSample = ((u16)buffer_rx[5] << 8) + buffer_rx[6];
          Send_Ack(iNEMO_SetOutMode, 1, 0x00);   
          }
        }
#else /* The AHRS Library is available */
        
        SetAhrs(buffer_rx[3] & 0x80 ? TRUE : FALSE);
        SetRawData(buffer_rx[3] & 0x20 ? TRUE : FALSE);
        s_uOutSelect=buffer_rx[3];   
        if(GetAhrs())
          s_uTimerFrequence = 0x03;
        else
          s_uTimerFrequence = (buffer_rx[4] & 0x38) >> 3;
        s_uOutMode =(buffer_rx[4] & 0x03);
        if((s_uTimerFrequence>7) || (s_uOutMode>0))
        {
          Send_Nack(iNEMO_SetOutMode,ValueOutOfRange);
        }     
        else
        {
          /* set timer */          
          Set_Timer(s_uTimerFrequence);
          /* set number of sample or continuos mode (s_nLengthSample=0) */
          s_nLengthSample = ((u16)buffer_rx[5] << 8) + buffer_rx[6];
          /*Select output destination*/
          Send_Ack(iNEMO_SetOutMode, 1, 0x00);  
        }                   
        
#endif  /*AHRS_MOD*/        
      }
      else
        Send_Nack(iNEMO_SetOutMode,WrongSyntax);
      break;  
      
      /* Get iNEMO output settings*/
    case iNEMO_GetOutMode:
      /* syntactic check :
      Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1
      nFrameSize --> check il the payload lenght is correct
      Ack response
      buffer_rx[3]  --> Enable setting 1 Enable/0 Disable AHRS |RFU 0 | Cal/Raw | Acc | Gyro | Mag | Press | Temp |
      buffer_rx[4]  --> Acquition Frequency + output type : 00 FQ2 FQ1 FQ0 OT2 OT1 OT0 
      buffer_rx[5]  --> Number of Samples (MSB)
      buffer_rx[6]  --> Number of Samples (LSB)
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2 ) 
      {
        u8 temp[4];
        temp[0]=s_uOutSelect;
        temp[1]=((s_uTimerFrequence ) << 3) + (s_uOutMode & 0x07);
        temp[2]=(u8)(s_nLengthSample >> 8);
        temp[3]=(u8)s_nLengthSample;    
        Send_Ack(iNEMO_GetOutMode, 5, temp);
      }
      else
        Send_Nack(iNEMO_GetOutMode,WrongSyntax);
      break;          
      
      /* start iNEMO acquisition data*/   
    case iNEMO_Start_Acquisition:
      /* syntactic check :
      Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1
      nFrameSize --> check il the payload lenght is correct
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2) 
      {
        s_iC=0;
        Send_Ack(iNEMO_Start_Acquisition, 1, 0x00);
        Enable_Timer(ENABLE);
      }
      else
        Send_Nack(iNEMO_Start_Acquisition,WrongSyntax);
      break;  
      
      /* start iNEMO acquisition data*/   
    case iNEMO_Stop_Acquisition:
      /* syntactic check :
      Frame Control -> Control frame; Ack required; Last fragment ; version 1; Normal Priority 
      Lenght -> 1
      nFrameSize --> check il the payload lenght is correct
      */          
      if(buffer_rx[0]==CTRL_wACK_LF && buffer_rx[1]==1  && nFrameSize== buffer_rx[1]+2) 
      {
        Enable_Timer(DISABLE);
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        s_iC=0;
        Send_Ack(iNEMO_Stop_Acquisition, 1, 0x00);
      }
      else
        Send_Nack(iNEMO_Stop_Acquisition,WrongSyntax);
      break;  
      
    default:
      Send_Nack(buffer_rx[2],CmdUnsupported);
      break;
    }
    
  }
  
}

/**
* Send a frame containing all sensor data and orientation data according to the transmission option specified by the CMD_START command.
*/
void DataProcess(u8 outmode, iNEMO_DATA *pData)
{
  u8 nByteToSend=0;  
  u8 databuffer[60]= {0};
  
  if ((s_iC < s_nLengthSample) ||  (s_nLengthSample ==0))
  {
    /* if AHRS is ENABLED*/
    if (GetAhrs())
    {

	  #ifdef  _GYRO_RPY      
	  //float roll, pitch, yaw;      
      LPRYxxxAL_Read_RawRate(pData->sGyro); 
      LSM303DLH_Acc_Read_RawData(pData->sAcc);
      LSM303DLH_Magn_Read_RawData(pData->sMag);      
	  #endif   
	     
      /* if accelerometer output is requested */
      if(outmode & 0x10) 
      {
        /*if raw data enabled*/
        if(GetRawData())
        {
         
          s16_to_u8_buffer(&(pData->sAcc[0]), &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&(pData->sAcc[1]), &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&(pData->sAcc[2]), &databuffer[nByteToSend + 6]);
        }
        /* Calibrated data are requested*/
        else
        {
          s16 acc[3];
          acc[0]=(s16)(pData->sAcc[0]/pData->uGain[0]) - pData->sOffset[0];
          acc[1]=(s16)(pData->sAcc[1]/pData->uGain[1]) - pData->sOffset[1];
          acc[2]=(s16)(pData->sAcc[2]/pData->uGain[2]) - pData->sOffset[2];                
          s16_to_u8_buffer(&acc[0], &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&acc[1], &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&acc[2], &databuffer[nByteToSend + 6]);
        }
        
        nByteToSend+=6;        
        
      }/* End of Accelerometer Data*/

	  #ifdef  _GYRO_RPY      
      /* if gyroscopes output is requested */      
      if(outmode & 0x08)
      {
        /*if raw data enabled*/        
        if(GetRawData())
        {
          s16_to_u8_buffer(&(pData->sGyro[0]), &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&(pData->sGyro[1]), &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&(pData->sGyro[2]), &databuffer[nByteToSend + 6]);
        }
        /* Calibrated data are requested*/        
        else
        {
          s16 dps[3];
          dps[0]=(s16)((pData->sGyro[0]/pData->uGain[3]) - pData->sOffset[3]);
          dps[1]=(s16)((pData->sGyro[1]/pData->uGain[4]) - pData->sOffset[4]);
          dps[2]=(s16)((pData->sGyro[2]/pData->uGain[5]) - pData->sOffset[5]);                
          s16_to_u8_buffer(&dps[0], &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&dps[1], &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&dps[2], &databuffer[nByteToSend + 6]);
        }
        
        nByteToSend+=6;             
      }/* End of Gyroscope Data */ 
      
      /* if magnetometer output is requested */     
      if(outmode & 0x04)
      {
        /*if raw data enabled*/        
        if(GetRawData())
        {  
          s16_to_u8_buffer(&(pData->sMag[0]), &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&(pData->sMag[1]), &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&(pData->sMag[2]), &databuffer[nByteToSend + 6]);
        }
        /* Calibrated data are requested*/
        else
        {
          s16 gauss[3]; 
          gauss[0]=(s16)(pData->sMag[0]*1000/pData->uGain[6]) - pData->sOffset[6];
          gauss[1]=(s16)(pData->sMag[1]*1000/pData->uGain[7]) - pData->sOffset[7];
          gauss[2]=(s16)(pData->sMag[2]*1000/pData->uGain[8]) - pData->sOffset[8];                
          s16_to_u8_buffer(&gauss[0], &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&gauss[1], &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&gauss[2], &databuffer[nByteToSend + 6]);   
        }
        
        nByteToSend+=6; 
      } /* End of Magnetometer Data */
	  #endif

      #ifdef _PRESS
      /* if pressure output is requested */       
      if(outmode & 0x02)
      {
        LPS001DL_Get_Raw_Pressure(&(pData->uPress));   
        /*if raw data enabled*/        
        if(GetRawData())
        { 
          u16_to_u8_buffer(&(pData->uPress), &databuffer[nByteToSend + 2]);
        }
        /* Calibrated data are requested*/ 
        else 
        {
          u16 p;
          p=(s16)((pData->uPress/pData->uGain[9]) - pData->sOffset[9]);
          u16_to_u8_buffer(&p, &databuffer[nByteToSend + 2]);             
        }
        
        nByteToSend+=2; 
      } /* End of Pressure Sensor Data */
      #endif

      /* if temperature output is requested */         
      if(outmode & 0x01)
      {
        /*if raw data enabled*/ 
        if(GetRawData())
        {
          STLM75_Read_Raw_Data(&(pData->sTemp));
          s16_to_u8_buffer(&(pData->sTemp),&databuffer[nByteToSend + 2]); 
        }
        /* Calibrated data are requested*/
        else
        {
          s16 p;
          STLM75_Read_Temperature_Signed(&(pData->sTemp)); 
          p=(s16)(pData->sTemp - pData->sOffset[10]);
          s16_to_u8_buffer(&p, &databuffer[nByteToSend + 2]); 
        }
        
        nByteToSend+=2;        
      }  /* End of Temeprature Sensor Data */  
      
#ifdef AHRS_MOD     
      
      pData->m_sensorData.m_fAcc[0]=((float)((*pData).sAcc[1]))*9.8/1000.0;
      pData->m_sensorData.m_fAcc[1]=((float)((*pData).sAcc[0]))*9.8/1000.0;
      pData->m_sensorData.m_fAcc[2]=-((float)((*pData).sAcc[2]))*9.8/1000.0;
      /* report gyro data into dps in float format */
      pData->m_sensorData.m_fGyro[1]=((float)((*pData).sGyro[0]))*(3.141592/180.0)/Gyro_SensitivityLSB_R_300dps;
      pData->m_sensorData.m_fGyro[0]=((float)((*pData).sGyro[1]))*(3.141592/180.0)/Gyro_SensitivityLSB_P_300dps; 
      pData->m_sensorData.m_fGyro[2]=-((float)((*pData).sGyro[2]))*(3.141592/180.0)/Gyro_SensitivityLSB_Y_300dps;
      
      pData->m_sensorData.m_fMag[1]=((float)((*pData).sMag[0]))/LSM_Magn_Sensitivity_XY_1_3Ga;
      pData->m_sensorData.m_fMag[0]=((float)((*pData).sMag[1]))/LSM_Magn_Sensitivity_XY_1_3Ga; 
      pData->m_sensorData.m_fMag[2]=-((float)((*pData).sMag[2]))/LSM_Magn_Sensitivity_Z_1_3Ga;
      
      iNEMO_AHRS_Update(&(pData->m_sensorData), &(pData->m_angle), &(pData->m_quat));
      roll  = pData->m_angle.m_fRoll* 180.0f / 3.141592f;
      pitch = pData->m_angle.m_fPitch * 180.0f / 3.141592f;
      yaw  = pData->m_angle.m_fYaw * 180.0f / 3.141592f;
      Float_To_Buffer(roll, &databuffer[nByteToSend + 2]);
      Float_To_Buffer(pitch, &databuffer[nByteToSend + 6]);
      Float_To_Buffer(yaw, &databuffer[nByteToSend +10]);
      Float_To_Buffer((pData->m_quat[0]), &databuffer[nByteToSend + 14]);
      Float_To_Buffer((pData->m_quat[1]), &databuffer[nByteToSend + 18]);
      Float_To_Buffer((pData->m_quat[2]), &databuffer[nByteToSend + 22]);
      Float_To_Buffer((pData->m_quat[3]), &databuffer[nByteToSend + 26]);
      nByteToSend+=4*sizeof (float) + 3* sizeof (float);// 4*float quaternions + 3*float angles.
#endif 
    }  /* End if AHRS is ENABLED*/
    
    /*AHRS DISABLE*/
    else
    {  
      if(outmode & 0x10) 
      {
        LSM303DLH_Acc_Read_RawData(pData->sAcc);
        /*if raw data enabled*/
        if(GetRawData())
        {
          s16_to_u8_buffer(&(pData->sAcc[0]), &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&(pData->sAcc[1]), &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&(pData->sAcc[2]), &databuffer[nByteToSend + 6]);
        }
        /* Calibrated data are requested*/
        else
        {
          s16 acc[3];
          acc[0]=(s16)(pData->sAcc[0]/pData->uGain[0]) - pData->sOffset[0];
          acc[1]=(s16)(pData->sAcc[1]/pData->uGain[1]) - pData->sOffset[1];
          acc[2]=(s16)(pData->sAcc[2]/pData->uGain[2]) - pData->sOffset[2];                
          s16_to_u8_buffer(&acc[0], &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&acc[1], &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&acc[2], &databuffer[nByteToSend + 6]);
        }
        
        nByteToSend+=6;        
        
      }/* End of Accelerometer Data*/
      
      /* if gyroscopes output is requested */
	  #ifdef  _GYRO_RPY      
      if(outmode & 0x08)
      {
        LPRYxxxAL_Read_RawRate(pData->sGyro); 
        /*if raw data enabled*/        
        if(GetRawData())
        {
          s16_to_u8_buffer(&(pData->sGyro[0]), &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&(pData->sGyro[1]), &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&(pData->sGyro[2]), &databuffer[nByteToSend + 6]);
        }
        /* Calibrated data are requested*/        
        else
        {
          s16 dps[3];
          dps[0]=(s16)((pData->sGyro[0]/pData->uGain[3]) - pData->sOffset[3]);
          dps[1]=(s16)((pData->sGyro[1]/pData->uGain[4]) - pData->sOffset[4]);
          dps[2]=(s16)((pData->sGyro[2]/pData->uGain[5]) - pData->sOffset[5]);                
          s16_to_u8_buffer(&dps[0], &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&dps[1], &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&dps[2], &databuffer[nByteToSend + 6]);
        }
        
        nByteToSend+=6;             
      }/* End of Gyroscope Data */ 
      
      /* if magnetometer output is requested */     
      if(outmode & 0x04)
      {
        LSM303DLH_Magn_Read_RawData(pData->sMag);
        /*if raw data enabled*/        
        if(GetRawData())
        {  
          s16_to_u8_buffer(&(pData->sMag[0]), &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&(pData->sMag[1]), &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&(pData->sMag[2]), &databuffer[nByteToSend + 6]);
        }
        /* Calibrated data are requested*/
        else
        {
          s16 gauss[3]; 
          gauss[0]=(s16)(pData->sMag[0]*1000/pData->uGain[6]) - pData->sOffset[6];
          gauss[1]=(s16)(pData->sMag[1]*1000/pData->uGain[7]) - pData->sOffset[7];
          gauss[2]=(s16)(pData->sMag[2]*1000/pData->uGain[8]) - pData->sOffset[8];                
          s16_to_u8_buffer(&gauss[0], &databuffer[nByteToSend + 2]);
          s16_to_u8_buffer(&gauss[1], &databuffer[nByteToSend + 4]);
          s16_to_u8_buffer(&gauss[2], &databuffer[nByteToSend + 6]);   
        }
        
        nByteToSend+=6; 
      } /* End of Magnetometer Data */
      #endif

	  #ifdef _PRESS
      /* if pressure output is requested */       
      if(outmode & 0x02)
      {
        LPS001DL_Get_Raw_Pressure(&(pData->uPress));   
        /*if raw data enabled*/        
        if(GetRawData())
        { 
          u16_to_u8_buffer(&(pData->uPress), &databuffer[nByteToSend + 2]);
        }
        /* Calibrated data are requested*/ 
        else 
        {
          u16 p;
          p=(s16)((pData->uPress/pData->uGain[9]) - pData->sOffset[9]);
          u16_to_u8_buffer(&p, &databuffer[nByteToSend + 2]);             
        }
        
        nByteToSend+=2; 
      } /* End of Pressure Sensor Data */
	  #endif      

      /* if temperature output is requested */         
      if(outmode & 0x01)
      {
        /*if raw data enabled*/ 
        if(GetRawData())
        {
          STLM75_Read_Raw_Data(&(pData->sTemp));
          s16_to_u8_buffer(&(pData->sTemp),&databuffer[nByteToSend + 2]); 
        }
        /* Calibrated data are requested*/
        else
        {
          s16 p;
          STLM75_Read_Temperature_Signed(&(pData->sTemp)); 
          p=(s16)(pData->sTemp - pData->sOffset[10]);
          s16_to_u8_buffer(&p, &databuffer[nByteToSend + 2]); 
        }
        
        nByteToSend+=2;        
      }  /* End of Temperature Sensor Data */  
    }
    
    s_iC++;
    databuffer[0]=(u8)(s_iC>>8);
    databuffer[1]=(u8)(s_iC);
    nByteToSend+=2; /* 2 byte counter*/
    
    Send_Data(iNEMO_Start_Acquisition, nByteToSend+1, databuffer);    
  }
  
  else { // end of data acquisition in Sample Mode.
    Enable_Timer(DISABLE);
  }
}


/**
*  Send Acknowledgment without Payload to the device, as correct reception of a frame.
* frame : Frame_Type to acknowledge.
*/
void Send_Ack(unsigned char frame, u8 length, u8* payload)
{
	u8 tmp[30];
	tmp[0] = ACK;
	tmp[1] = length;
        tmp[2] = frame;
        if(length>1)
          CopyBuffer(payload, &tmp[3],length-1);
	androidAccessoryWrite(tmp, length + 2);
}


/**
* Send Command w/o payload to the device, as correct reception of a frame.
* frame : Frame_Type to acknowledge.
*/
void Send_Data(unsigned char frame, u8 length, u8* payload)
{
	u8 tmp[100];
	tmp[0] = DATA;
	tmp[1] = length;
        tmp[2] = frame;
        if(length>1)
          CopyBuffer(payload, &tmp[3],length-1);
	androidAccessoryWrite(tmp, length + 2);
}


/**
*  Send NAcknowledgment without Payload to the device, as correct reception of a frame.
*  frame : Frame_Type to acknowledge.
*/
void Send_Nack(unsigned char frame, u8 error_code)
{
	u8 tmp[30];
	tmp[0] = NACK;
	tmp[1] = 2;
    tmp[2] = frame;
    tmp[3] = error_code;
	androidAccessoryWrite(&tmp[0], 4);
}

/**

 * Configure the timer in order to generate a recursive interrupt according to the
 * number parameter.
 * The interrupt is used by the iNemo Data task to send the sensor data at a given frequency.
 * LOW_FREQUENCY - specifies a frequency acquisition of 1Hz
 * MEDIUM_FREQUENCY_1 - specifies a frequency acquisition of 10Hz
 * MEDIUM_FREQUENCY_2 - specifies a frequency acquisition of 25Hz
 * HIGH_FREQUENCY - specifies a frequency acquisition of 50Hz. This is the
 * default value
 */

void Set_Timer(unsigned char number)
{
  unsigned short a;
  unsigned short b;
  unsigned long n;
  int  frequency; //This value is the timer frequency expressed in Hz
  // table to convert the frequency number to actual frequency value.
  const int nFreqEnum2freVal[7] = { 1, 10, 25, 50, 30,100, 400 };
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  frequency = number < 7 ? nFreqEnum2freVal[number] : 400;
  TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );

  // Time base configuration for timer 2 - which generates the interrupts.
  n = 72000000/frequency;

  prvFindFactors( n, &a, &b );
  TIM_TimeBaseStructure.TIM_Period = b - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = a - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );
  TIM_ARRPreloadConfig(TIM2, ENABLE );

  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 13;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );
}


/**

 * Start and stop the timer used by the iNemo Data Task.
 * command start the timer if ENABLE, stop the timer if DISABLE.
 */

void Enable_Timer(FunctionalState command) {
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig( TIM2, TIM_IT_Update, command );
	TIM_Cmd(TIM2, command);

}

/**
 * Set Output Mode option
 * outmode : 0 Disable/1 Enable order bit :AHRS | RFU 0 | Cal/Raw | Acceleration | Gyro | Magnetometer | Pressure | temperature 
 *                  Cal/Raw --> 0-->Calibrated   1-->Raw                
 *                  Calibrated data: Sensitivity and offset [mg, dps etc..] 
 *                  Raw data: [lsb]
 */
void SetOutMode(u8 outmode) {
  s_uOutSelect = outmode;
}

/**
 * Return output mode option
 */
u8 GetOutMode() {
  return s_uOutSelect;
}

/**
 * set a boolean to enable/disable  AHRS
 * bEnable :  TRUE AHRS enable FALSE AHRS disable
 */
void SetAhrs(bool bEnable) {
  s_bAhrsEnabled = bEnable;
}

/**
 * set a boolean to enable/disable  RAW DATA
 * bEnable :  TRUE RAW enable FALSE RAW disable
 */
void SetRawData(bool bEnable) {
  s_bRawDataEnabled = bEnable;
}


/**
 * return a boolean to check if AHRS is enabled/disabled
 * s_bAhrsEnabled : TRUE Ahrs enable FALSE Ahrs disable
 */
bool GetAhrs()
{
  return s_bAhrsEnabled;
}


/**
 *  return a boolean to check if Raw is enabled/disabled
 *  s_bAhrsEnabled : TRUE Ahrs enable FALSE Ahrs disable
 */
bool GetRawData()
{
  return s_bRawDataEnabled;
}

