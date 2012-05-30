#ifndef __COM_MANAGER_H
#define __COM_MANAGER_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "stm32f10x_map.h"
#include "iNEMO_lib.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

#define FRAME_SIZE 64

/**
* FRAME_CONTROL_DEFINE
* 
*/
#define CTRL_type            0x00 /*!< control frame */
#define DATA_type            0x40 /*!< data frame */
#define ACK_type             0x80 /*!< Ack frame */
#define NACK_type            0xC0 /*!< NACK frame */

#define ACK_req              0x20 /*!< Ack required */
#define ACK_NOTreq           0x00 /*!< NACK required */

#define Last_Frag            0x00 /*!<  Last Fragment */
#define More_Frag            0x10 /*!< More Fragment */

#define Version_1            0x00 /*!< Frame Version */

#define QoS_Normal           0x00 /*!< Data no Ack Last Fragment */
#define Qos_Medium           0x01 /*!< Data with Ack More Fragment */
#define QoS_High             0x02 /*!< Ack no payload */


#define Ctrl_Check(type,ack,frag,vers,QoS) (type | ack | frag | vers | QoS) /*!< macro to build control byte */
#define CTRL_wACK_LF  Ctrl_Check(CTRL_type, ACK_req ,Last_Frag ,Version_1 ,QoS_Normal)  /*!< macro to build control frame with ack , last fragment */
#define CTRL_noACK_LF  Ctrl_Check(CTRL_type, ACK_NOTreq ,Last_Frag ,Version_1 ,QoS_Normal) /*!< macro to build control frame without ack , last fragment */
#define DATA Ctrl_Check(DATA_type, ACK_NOTreq ,Last_Frag ,Version_1 ,QoS_Normal) /*!< macro to build data frame with ack , last fragment */
#define ACK   Ctrl_Check(ACK_type, ACK_NOTreq, Last_Frag ,Version_1 ,QoS_Normal) /*!< macro to build Ack frame */
#define NACK   Ctrl_Check(NACK_type, ACK_NOTreq ,Last_Frag ,Version_1 ,QoS_Normal) /*!< macro to build NACK frame with ack  */
#define TRACE Ctrl_Check(DATA_type, ACK_NOTreq ,Last_Frag ,Version_1 ,Qos_Medium) /*!< macro to build data frame with ack , last fragment, QOS Medium */


/* end of group FRAME_CONTROL_DEFINE */


/**
*  ERROR_CODE
* 
*/

#define  CmdUnsupported     0x01
#define  ValueOutOfRange    0x02
#define  NotExecutable      0x03
#define  WrongSyntax        0x04
#define  iNEMONotConnected   0x05

/* end of group ERROR_CODE */


/**
*  MESSAGE_ID
* 
*/

/**
*  COMMUNICATION_CONTROL_FRAME
* 
*/

#define iNEMO_Connect                 0x00
#define iNEMO_Disconnect              0x01
#define iNEMO_Reset_Board             0x02
#define iNEMO_Enter_DFU_Mode          0x03
#define iNEMO_Trace                   0x07
#define iNEMO_Led                     0x08


/* end of group COMMUNICATION_CONTROL_FRAME */

/**
*  BOARD_INFO_FRAME
* 
*/ 


#define iNEMO_Get_Device_Mode         0x10
#define iNEMO_Get_MCU_ID              0x12
#define iNEMO_Get_FW_Version          0x13
#define iNEMO_Get_HW_Version          0x14
#define iNEMO_Identify                0x15
#define iNEMO_Get_AHRS_Library        0x17
#define iNEMO_Get_Libraries           0x18

/* end of group BOARD_INFO_FRAME */

/**
*  SENSOR_SETTING_FRAME
* 
*/

#define iNEMO_Set_Sensor_Parameter             0x20
#define iNEMO_Get_Sensor_Parameter             0x21
#define iNEMO_Restore_Default_Parameter                0x22

/* end of group SENSOR_SETTING_FRAME */


/**
*  ACQUISITION_SENSOR_DATA_FRAME
* 
*/

#define iNEMO_SetOutMode              0x50
#define iNEMO_GetOutMode              0x51
#define iNEMO_Start_Acquisition       0x52
#define iNEMO_Stop_Acquisition        0x53

/* end of group ACQUISITION_SENSOR_DATA_FRAME */

/* end of group MESSAGE_ID */



//  FREQUENCY ACQUISITION VALUES

#define LOW_FREQUENCY         0x00    /*!< 1 HZ frequency acquisition */
#define MEDIUM_FREQUENCY_1    0x01    /*!< 10 HZ frequency acquisition */
#define MEDIUM_FREQUENCY_2    0x02    /*!< 25 HZ frequency acquisition */
#define HIGH_FREQUENCY        0x03    /*!< 50 HZ frequency acquisition */


// FRAME TYPE for PC

/**
*  FW_HW_Version
* 
*/

#define iNEMO_FIRMWARE_VERSION	    "iNEMO Firmware_Version_2.2"
#define SIZE_FWversion  strlen(iNEMO_FIRMWARE_VERSION)

#define iNEMO_HARDWARE_VERSION	    "iNEMO V2 Hardware_Version_1"
#define SIZE_HWversion  strlen(iNEMO_HARDWARE_VERSION)

/* end of group FW_HW_Version */


/**
*  Libraries_Version
* 
*/ /*
#ifdef AHRS_MOD
#define iNEMO_AHRS_VERSION	    "iNEMO AHRS ENABLE V1.2.0"
#define iNEMO_AHRS_LIBRARY          0x01
#else
#define iNEMO_AHRS_VERSION          "iNEMO AHRS NOT AVAILABLE"
#define iNEMO_AHRS_LIBRARY	    0x00
#endif
#define SIZE_AHRSlibrary  strlen(iNEMO_AHRS_VERSION)

#ifdef COMPASS_MOD
#define iNEMO_COMPASS_LIBRARY        0x02
#else
#define iNEMO_COMPASS_LIBRARY	     0x00
#endif

#ifdef ALTIMETER_MOD
#define iNEMO_ALTIMETER_LIBRARY      0x04
#else
#define iNEMO_ALTIMETER_LIBRARY	     0x00
#endif

#ifdef TRACE_MOD
#define iNEMO_TRACE_LIBRARY          0x08
#else
#define iNEMO_TRACE_LIBRARY	     0x00
#endif

#ifdef FAT_MOD
#define iNEMO_FAT_LIBRARY            0x10
#else
#define iNEMO_FAT_LIBRARY	     0x00
#endif


#define AVAILABLE_LIBRARIES     (iNEMO_AHRS_LIBRARY | iNEMO_COMPASS_LIBRARY | \
                                  iNEMO_ALTIMETER_LIBRARY | iNEMO_TRACE_LIBRARY |\
                                  iNEMO_FAT_LIBRARY)*/ /*!< macro to build available libraries */

/* end of group Libraries_Version */

struct receivedMsg{
	int	length;
	u8	msg[FRAME_SIZE];
};

void Send_Ack(unsigned char frame, u8 length, u8* payload);
void Send_Nack(unsigned char frame, u8 error_code);
void Send_Data(unsigned char frame, u8 length, u8* payload);
void Set_Timer(unsigned char number);
void ParseCommandFrame(u32 nFrameSize, u8* buffer_rx, iNEMO_DATA * pdata);
void DataProcess(u8 outmode, iNEMO_DATA *pData);

void SetAhrs(bool bEnable);
void SetRawData(bool bEnable);
bool GetAhrs();
bool GetRawData();
void SetOutMode(u8 outmode);
u8 GetOutMode();

static void Enable_Timer(FunctionalState command);

#endif /*__COM_MANAGER_H */
