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


void ledOn(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
}


void ledOff(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);	
}


int inemoUtilInit(void)
{
	uartHandle = xSerialPortInitMinimal(115200, 64);
	if(uartHandle != NULL){
		print("UART initialized succesfully\n\r");
		return 0;
	}

	return -1;
}


int strlen(char* string)
{
	int len = 0;

	while(string[len] != 0){
		len++;
	}
	return len;
}


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


void panic(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
	for(;;);
}
