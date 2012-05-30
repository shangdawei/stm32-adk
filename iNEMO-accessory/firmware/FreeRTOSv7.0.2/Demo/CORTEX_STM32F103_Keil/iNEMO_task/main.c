/** iNEMO ADKPing Accessory firmware
 * David Siorpaes (C) STMicroelectronics 2011
 * 
 * Sends a string to Android application and receives back
 * data from Android application.
 *
 * Derived from Android DemoKit.pde Arduino application
 */

			
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "iNEMO_util.h"
#include <AndroidAccessory.h>
#include "comManager.h"

#include "stm32f10x_tim.h"
#include "STLM75.h"
#include "LSM303DLH.h"
#include "LPS001DL_I2C.h"
#include "LPRYxxxAL.h"
#include <stdio.h>

/**
 * Pointer to iNemoData structure. This struct contatins
 * the setting values for the sensor parameters	of the board.
 * (see iNEMO_DATA definition in iNEMO_lib.h)
 * 
 */
static iNEMO_DATA *s_pSharedData = NULL;


/**
 * This resource is used to synchronize the iNemo Command Manager task
 * with the Accessory task.
 */
static xQueueHandle s_usbQueue;


/**
 * This resource is used to synchronize the TIM2 IRQ and the iNemo data acquisition task.
 */
static xSemaphoreHandle s_timSemaphore;


/* Forward declaration */
static void accessoryTask(void *pvParameters);
static void inCommandTaskFunction(void *pvParameters);
static void inDataTaskFunction(void *pvParameters);
static void Timer_Config(void);					 


int main( void )
{  
 	int err;

	/* Initialize board */
  	prvSetupHardware();
	Timer_Config();
	gpiosInit();
	
	/* Create the queue used to synchronize iNemo Command Manager task and the accessory task. */
	s_usbQueue = xQueueCreate(1, sizeof(struct receivedMsg));
	if (!s_usbQueue) {
		// Error in resource creation.
		print("Error in resource creation: s_usbQueue\n");
  		panic();
	}

	/* Create the semaphore used to synchronize  the iNemo data acquisition task and the TIM2 interrupt service routine.  */
	vSemaphoreCreateBinary(s_timSemaphore);
	if (!s_timSemaphore) {
		// Error in resource creation.
		print("Error in resource creation: s_timSemaphore\n");
  		panic();
	}
        xSemaphoreTake(s_timSemaphore, 0);

	/* Create Android Accessory task */
  	if( xTaskCreate(accessoryTask, (signed portCHAR*) "ADK", 512, NULL, IN_TASK_PRIORITY + 1, NULL ) != pdPASS ) {
		print("Error in task creation: AccessoryTask\n");
  		panic();
	}
	
	/* Create the iNemo Command task */
	if ( xTaskCreate(inCommandTaskFunction, "iNemoCmd", IN_TASK_STACK_SIZE, NULL, IN_TASK_PRIORITY+1, NULL) != pdPASS ) {
		// Error in task creation
		print("Error in task creation: iNemoCmd\n");
  		panic();
	}
	
	/* Create the iNemo Data stream task */
	if ( xTaskCreate(inDataTaskFunction, "iNemoData", IN_TASK_STACK_SIZE, NULL, IN_TASK_PRIORITY, NULL) != pdPASS ) {
		// Error in task creation
		print("Error in task creation: iNemoData\n");
  		panic();
	}
	
	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was not enough heap space to create the idle task. */
	panic();

	return 0;
}


void accessoryTask(void* pvParameters)
{
	/* Initialize iNEMO utility library */
 	inemoUtilInit();

	/* Construct accessory. Vendor, application and version MUST match 
	 * with Android application manifest correspondent entries */
	AndroidAccessory("STMicroelectronics", "adkping", "Just pings data", "2.0",
                        "http://www.st.com", "1234567890123456");

	/* Power on accessory */
	androidAccessoryPowerOn();

	delay(200);

	/* Main loop */
	while(1){

		/* Checks if there is a device connected. If there is
		 * one, it tries to switch the device in Accesory mode.
		 * If there is not any device attached, or the device
		 * does not support ADK (or it simply refuses the connection),
		 * false is returned. */
		if (androidAccessoryIsConnected()) {
			int len = 0;
			u8 buffer_rx[FRAME_SIZE];
        	print("Inside acc.isconnected..\r\n");
			/* Reads the input stream. If the device sent a message to the board,
			* the message is stored in buffer_rx and the number of bytes read 
			* are stored in len. If no message is present len will be -1. */
			len =  androidAccessoryRead(buffer_rx, sizeof(buffer_rx), 1);
			if(len > 0) {
				/* The variable msg will contain the message received from
				* the device and its length (in bytes). It is enqueued to 
				* s_usbQueue in order to be processed by the Command
				* Manager Task. */
				struct receivedMsg msg;
				msg.length = len;
				memcpy(msg.msg, buffer_rx, FRAME_SIZE); 
				xQueueSend(s_usbQueue, &msg, (portTickType) 0);
			}
			delay(20);
    	} else{
	    	print("Test loop...\r\n");
			delay(20);
			ledOn();
			delay(20);
			ledOff();
		}
	}
}

/**
 * iNemo Command Manager task control function.
 * This task waits for a frame coming from the Accessory Task.
 * 
 * pvParameters not used.
 */		 
void inCommandTaskFunction(void *pvParameters) {
	struct receivedMsg msg;

	for (;;) {
		if ( xQueueReceive(s_usbQueue, &msg, portMAX_DELAY ) == pdTRUE ) {
			/* Process the message received from the device. */
			ParseCommandFrame(msg.length, msg.msg,  s_pSharedData);
		}
	}
}		


/**
 * iNemo data acquisition/send task control function.
 * This task is synchronized with the  Timer2 interrupt. It reads all sensor data, packages the data according to the frame
 * format and sends the data to the device over the ADK connection.
 *
 * pvParameters not used.
 */		   

void inDataTaskFunction(void *pvParameters)
{
        iNEMO_DATA data;
        iNEMO_Data_Init(&data);

        s_pSharedData = &data;

#ifdef AHRS_MOD
	iNEMO_AHRS_Init(&data.m_sensorData, &data.m_angle, &data.m_quat);
#endif

	for (;;) {
		if ( xSemaphoreTake(s_timSemaphore, portMAX_DELAY) == pdTRUE ) {
                    DataProcess(GetOutMode(),&data);
		}
	}
}			 

/**
 * Configures the timer 2
 */			   

void Timer_Config(void) {
  unsigned short a;
  unsigned short b;
  unsigned long n;
  unsigned char frequency = 50; //This value is the frequency interrupts in Hz
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  // Enable timer clocks
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );

  // Time base configuration for timer 2 - which generates the interrupts.
  n = configCPU_CLOCK_HZ/frequency;
  prvFindFactors( n, &a, &b );
  TIM_TimeBaseStructure.TIM_Period = b - 1;
  TIM_TimeBaseStructure.TIM_Prescaler = a - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure );
  TIM_ARRPreloadConfig( TIM2, ENABLE );

  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 13;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init( &NVIC_InitStructure );
}				 

/**
 * This function handles TIM2 global interrupt request by resuming the
 * iNemoData task. The timer establishes the sending data rate.
 */				   

void TIM2_IRQHandler(void) {
	if(TIM_GetITStatus(TIM2, TIM_IT_Update)) {
    	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	    xSemaphoreGiveFromISR(s_timSemaphore, &xHigherPriorityTaskWoken);
	    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
  }
}					 

