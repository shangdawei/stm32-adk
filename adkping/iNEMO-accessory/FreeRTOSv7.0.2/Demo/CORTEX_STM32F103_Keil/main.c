/** iNEMO ADK Accessory firmware
 */

			
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"	 
#include "semphr.h"
#include "serial.h"		  

/* Library includes. */
#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_spi.h"

#include <AndroidAccessory.h>

#include "Max3421e.h"
#include "Max3421e_constants.h"

#include "inemoutil.h"


char sendBuffer[]="ArduinoAccessory!";
char receiveBuffer[128];



void mainPhase()
{
	int len;
  	while(1){    
        len = androidAccessoryWrite(sendBuffer, 17);
        print("Sent accessory name... Total chars sent: ");
        printHex(len);
		print("\r\n");
        print("Received from accessory: ");
        len = androidAccessoryRead(receiveBuffer, sizeof(receiveBuffer), 1);
        print(receiveBuffer);
        print("\r\n");
		print(" Total chars received: ");
        printHex(len);
		print("\r\n");
        delay(1000);
  }      
}


void accessoryTask(void* params)
{
	/* Initialize iNEMO utility library */
 	inemoUtilInit();

	/* Construct accessory */
	AndroidAccessory("STMicroelectronics", "adkping", "Just pings data", "2.0",
                        "http://www.st.com", "1234567890123456");

	/* Power on accessory */
	androidAccessoryPowerOn();

	delay(200);

	/* Main loop */
	while(1){
		if (androidAccessoryIsConnected()) {
        	print("Inside acc.isconnected..\r\n");
            mainPhase();
            print("Exited from mainphase...\r\n");
    	}

    	print("Test loop...\r\n");
		delay(20);
		ledOn();
		delay(20);
		ledOff();
	}
}


int toggle = 0;

void vTimer2IntHandler( void )
{
	/* ACK interrupt */
    TIM_ClearITPendingBit( TIM2, TIM_IT_Update );

	/* Read back the IRQ status to avoid double IRQ race */
	/* See https://my.st.com/public/STe2ecommunities/mcu/Lists/ARM%20CortexM3%20STM32/Flat.aspx?RootFolder=%2Fpublic%2FSTe2ecommunities%2Fmcu%2FLists%2FARM%20CortexM3%20STM32%2FTimer%20update%20event%20interrupt%20retriggering%20after%20exit&FolderCTID=0x01200200770978C69A1141439FE559EB459D758000626BE2B829C32145B9EB5739142DC17E&currentviews=241 */
	/* See https://my.st.com/public/FAQ/Lists/faqlist/DispForm.aspx?ID=144&level=1&objectid=141&type=product&Source=%2fpublic%2fFAQ%2ffaq.aspx%3flevel%3d1%26objectid%3d141%26type%3dproduct */
	TIM_GetITStatus(TIM2, TIM_IT_Update);

	if(toggle++ % 2)
		GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
	else
		GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);
}


int main( void )
{  
 	int err;

  	prvSetupHardware();
	gpiosInit();

  	err = xTaskCreate(accessoryTask, (signed portCHAR*) "ADK", 512, NULL, tskIDLE_PRIORITY + 1, NULL );
  	if(err != pdPASS)
  		panic();

	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was not enough heap space to create the
	idle task. */
	panic();

	return 0;
}


