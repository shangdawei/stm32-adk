/*
    FreeRTOS V7.0.2 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Fast Interrupt Test" - A high frequency periodic interrupt is generated
 * using a free running timer to demonstrate the use of the
 * configKERNEL_INTERRUPT_PRIORITY configuration constant.  The interrupt
 * service routine measures the number of processor clocks that occur between
 * each interrupt - and in so doing measures the jitter in the interrupt timing.
 * The maximum measured jitter time is latched in the ulMaxJitter variable, and
 * displayed on the LCD by the 'Check' task as described below.  The
 * fast interrupt is configured and handled in the timertest.c source file.
 *
 * "LCD" task - the LCD task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the LCD send the message on a queue to the LCD task instead of
 * accessing the LCD themselves.  The LCD task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" task -  This only executes every five seconds but has the highest
 * priority so is guaranteed to get processor time.  Its main function is to
 * check that all the standard demo tasks are still operational.  Should any
 * unexpected behaviour within a demo task be discovered the 'check' task will
 * write an error to the LCD (via the LCD task).  If all the demo tasks are
 * executing with their expected behaviour then the check task writes PASS
 * along with the max jitter time to the LCD (again via the LCD task), as
 * described above.
 *
 */

#define BOARD_IS_INEMOV2  1

/* Standard includes. */
#include <stdio.h>

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


/* From Arduino USB firmware */

#define rUSBIRQ     0x68    //13<<3
/* USBIRQ Bits  */
#define bmVBUSIRQ   0x40    //b6
#define bmNOVBUSIRQ 0x20    //b5
#define bmOSCOKIRQ  0x01    //b0
#define rUSBCTL     0x78    //15<<3
/* USBCTL Bits  */
#define bmCHIPRES   0x20    //b5
#define bmPWRDOWN   0x10    //b4

#define rPINCTL     0x88    //17<<3
/* PINCTL Bits  */
#define bmFDUPSPI   0x10    //b4
#define bmINTLEVEL  0x08    //b3
#define bmPOSINT    0x04    //b2
#define bmGPXB      0x02    //b1
#define bmGPXA      0x01    //b0

#define rREVISION   0x90    //18<<3

/*
 * Configure the clocks, GPIO and other peripherals as required by the demo.
 */
static void prvSetupHardware( void );

xSemaphoreHandle xBinarySemaphore;
xComPortHandle uartHandle;

u8 revision1;
u8 revision2;
u8 mydata1;
u8 mydata2;

void panic(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
	for(;;);
}


void gpiosInit(void)
{
	GPIO_InitTypeDef gpioInit;
	EXTI_InitTypeDef extiInit;
    NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable iNemo LED PB9 */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_9;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpioInit);

	/* Enable PB0 for output */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_0;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &gpioInit);

	/* Enable PC7 for output */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_7;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &gpioInit);

	/* Enable PC3 for output */
	GPIO_StructInit(&gpioInit);
	gpioInit.GPIO_Pin = GPIO_Pin_3;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &gpioInit);
}


void regWr(u8 reg, u8 val)
{
	/* Slave select low */
	//vTaskDelay(10);
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET);
	vTaskDelay(1);

	/* Send command: since we are writing set command bit accordingly */
	reg |= 0x02;
	SPI_SendData(SPI1, reg);
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET){}
	
	/* Dummy read */
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE)== RESET){}
  	SPI_ReceiveData(SPI1);
		
	/* Send value */
	SPI_SendData(SPI1, val);
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET){}

	/* Dummy read */
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE)== RESET){}
  	SPI_ReceiveData(SPI1);

	/* Slave select hi */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET);
	vTaskDelay(1);
}			 

/* Cfr:  LCD_ReadReg */
u8 regRd(u8 reg)
{
	u8 rddata;

	/* Slave select low. According to Maxim spec (p13), we should wait at
	 * least tl=30ns */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET);
	vTaskDelay(1);
	
	/* Send command */
	SPI_SendData(SPI1, reg);
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET){}

	/* Dummy read */
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE)== RESET){}
  	SPI_ReceiveData(SPI1);	

	/* Dummy write */
	SPI_SendData(SPI1, 0xFF);
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET){}
  	
	/* Actual read */
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE)== RESET){}
  	rddata = SPI_ReceiveData(SPI1);

	/* Slave select hi. Should wait tcsw=300 ns before next transfer */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET);
	vTaskDelay(1);

	return rddata;
}

u8 testval;

void spiTask(void* params)
{	
	
	/* SS high */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET);
	vTaskDelay(1);

	/* Set Full Duplex mode: 0x8a (TX), 0x1a (TX)*/
	regWr(rPINCTL, bmFDUPSPI + bmINTLEVEL + bmGPXB);
	//regWr(rPINCTL, bmFDUPSPI);

	/* Reset */
	//regWr( rUSBCTL, bmCHIPRES );                        //Chip reset. This stops the oscillator
    //regWr( rUSBCTL, 0x00 );                             //Remove the reset
	//while(!(regRd( rUSBIRQ ) & bmOSCOKIRQ )){} 			//wait until the PLL is stable

	/* Read revision: 0x90(TX), 0x12 (or 0x48) (RX) */
	revision1 = regRd(rREVISION);
	revision2 = regRd(rREVISION);
	mydata1 = regRd(rPINCTL);
	mydata2 = regRd(rPINCTL);

	regWr(20<<3, 0xb);
	testval = regRd(20<<3);

	uartHandle = xSerialPortInitMinimal(115200, 32);

	while(1){	
		/* LED activity */
		GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
		vTaskDelay(200);
		GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);
		vTaskDelay(200);

		vSerialPutString(uartHandle, "bella zia ", 10);
	}
}


void vTimer2IntHandler( void )
{
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* 'Give' the semaphore to unblock the task. */
	xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );

	if( xHigherPriorityTaskWoken == pdTRUE )
	{
		/* Giving the semaphore unblocked a task, and the priority of the
		unblocked task is higher than the currently running task - force
		a context switch to ensure that the interrupt returns directly to
		the unblocked (higher priority) task.

		NOTE: The syntax for forcing a context switch is different depending
		on the port being used.  Refer to the examples for the port you are
		using for the correct method to use! */
		taskYIELD();
	}

	/* ACK interrupt */
    TIM_ClearITPendingBit( TIM2, TIM_IT_Update );

	/* Read back the IRQ status to avoid double IRQ race */
	/* See https://my.st.com/public/STe2ecommunities/mcu/Lists/ARM%20CortexM3%20STM32/Flat.aspx?RootFolder=%2Fpublic%2FSTe2ecommunities%2Fmcu%2FLists%2FARM%20CortexM3%20STM32%2FTimer%20update%20event%20interrupt%20retriggering%20after%20exit&FolderCTID=0x01200200770978C69A1141439FE559EB459D758000626BE2B829C32145B9EB5739142DC17E&currentviews=241 */
	/* See https://my.st.com/public/FAQ/Lists/faqlist/DispForm.aspx?ID=144&level=1&objectid=141&type=product&Source=%2fpublic%2fFAQ%2ffaq.aspx%3flevel%3d1%26objectid%3d141%26type%3dproduct */
	TIM_GetITStatus(TIM2, TIM_IT_Update);
}


int SPI_Config()
{
	SPI_InitTypeDef    SPI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;	

  	/* Enable SPI1 clock  */
  	RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  	/* Configure SPI1 pins: NSS, SCK, MISO and MOSI */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* SPI1 Config (cfr. Demo/Common/drivers/ST/STM32F10xFWLib/src/lcd.c) */
  	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  	//SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  	SPI_Init(SPI1, &SPI_InitStructure);

  	/* SPI2 enable */
  	SPI_Cmd(SPI1, ENABLE);

	return 0;
}


int main( void )
{  
 	int err;


  	prvSetupHardware();
	gpiosInit();
	SPI_Config();
   	
	
  	err = xTaskCreate(spiTask, (signed portCHAR*) "SPI", 256, NULL, tskIDLE_PRIORITY + 1, NULL );
  	if(err != pdPASS)
  		panic();

	/* Start the scheduler. */
	vTaskStartScheduler();
	
	/* Will only get here if there was not enough heap space to create the
	idle task. */
	panic();

	return 0;
}


static void prvSetupHardware( void )
{
	/* Start with the clocks in their expected state. */
	RCC_DeInit();

	/* Enable HSE (high speed external clock). */
	RCC_HSEConfig( RCC_HSE_ON );

	/* Wait till HSE is ready. */
	while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
	{
	}

	/* 2 wait states required on the flash. */
	*( ( unsigned portLONG * ) 0x40022000 ) = 0x02;

	/* HCLK = SYSCLK */
	RCC_HCLKConfig( RCC_SYSCLK_Div1 );

	/* PCLK2 = HCLK */
	RCC_PCLK2Config( RCC_HCLK_Div1 );

	/* PCLK1 = HCLK/2 */
#if BOARD_IS_INEMOV2
	RCC_PCLK1Config( RCC_HCLK_Div2 );	  //STMF103	Max 32MHz
#elif BOARD_IS_DISCOVERY
	RCC_PCLK1Config( RCC_HCLK_Div1 );	  //STMF100	Max 24MHz
#else
#error "Please define either BOARD_IS_INEMOV2 or BOARD_IS_DISCOVERY"
#endif
	
	/* PLLCLK = 8MHz * 9 = 72 MHz. */
#if BOARD_IS_INEMOV2
	RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 ); //STMF103 @72MHz
#elif BOARD_IS_DISCOVERY
	RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_3 );	 //STMF100 @24MHz
#else
#error "Please define either BOARD_IS_INEMOV2 or BOARD_IS_DISCOVERY"
#endif

	/* Enable PLL. */
	RCC_PLLCmd( ENABLE );

	/* Wait till PLL is ready. */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

	/* Wait till PLL is used as system clock source. */
	while( RCC_GetSYSCLKSource() != 0x08 )
	{
	}

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO
							| RCC_APB2Periph_ALL , ENABLE );

	/* SPI2 Periph clock enable */
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2 | RCC_APB1Periph_ALL, ENABLE );


	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

	/* Enable prefetch */
	*((unsigned long*)(0x40022000)) = 0x12;
}

