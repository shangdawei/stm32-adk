#include <string.h>

#include "inemoutil.h"

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

#include "inemoutil.h"

xComPortHandle uartHandle;

/* Turns iNEMO D3 led on */
void ledOn(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
}

/* Turns iNEMO D3 led off */
void ledOff(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);	
}


/* Innitializes UART. MUST be called after FreeRTOS scheduler has started */
int inemoUtilInit(void)
{
	uartHandle = xSerialPortInitMinimal(115200, 256);
	if(uartHandle != NULL){
		print("UART initialized succesfully\n\r");
		return 0;
	}

	return -1;
}


/* Prints on serial line a string */
int print(char* string)
{
	int len;

	len = strlen(string);
	if(uartHandle){
		vSerialPutString(uartHandle, (const signed char*)string, len);
		return len;
	}
	else{
		return -1;
	}
}


/* Prints on serial line hex value */
void printHex(unsigned int val)
{
	char string[5];
	u8 i;
	unsigned int rem;

	for(i=0; i<4;i++){
		rem = val % 16;
		if(rem >= 10)
			string[3-i] = 'A' + rem - 10;
		else
			string[3-i] = '0' + rem;
		val /= 16;
	}
	
	string[4] = 0;
	print(string);
}


/* This mimics the Arduino millis() call: returns the number of milliseconds
 * since system reset */
unsigned int millis(void)
{
	return xTaskGetTickCount()/portTICK_RATE_MS;
}

/* Arduino adapter: waits for  given amount of time (milliseconds) */
void delay(int delay)
{
	vTaskDelay(delay);
}


/* Call this when things go awfully wrong */
void panic(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
	for(;;);
}
