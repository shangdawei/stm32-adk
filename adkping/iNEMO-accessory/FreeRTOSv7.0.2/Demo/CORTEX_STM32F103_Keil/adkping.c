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
#include "inemoutil.h"
#include <AndroidAccessory.h>

/* String that accessory sends to Android application */
char sendBuffer[]="ArduinoAccessory!";

/* Buffer that holds data sent back from Android application */
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

	/* Construct accessory. Vendor, application and version MUST match 
	 * with Android application manifest correspondent entries */
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


int main( void )
{  
 	int err;

	/* Initialize board */
  	prvSetupHardware();
	gpiosInit();

	/* Spawn ADK task */
  	err = xTaskCreate(accessoryTask, (signed portCHAR*) "ADK", 512, NULL, tskIDLE_PRIORITY + 1, NULL );
  	if(err != pdPASS)
  		panic();

	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was not enough heap space to create the idle task. */
	panic();

	return 0;
}

