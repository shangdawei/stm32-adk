/** USB shield support library. Ported from Google code base "ADK_release_0512"
 * 
 * Public methods are preceded with "max3421e" prefix.
 * Private methods signatures are left unchanged 
 */ 

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

#include "Max3421e.h"


/* Public methods */
void Max3421e(void)
{
	spi_init();
	pinInit();
}

void max3421eRegWr(u8 reg, u8 val)
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
u8 max3421eRegRd(u8 reg)
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



/* Private methods */
void spi_init(void)
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
}

void pinInit(void)
{
	/* Slave select high */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET);
	vTaskDelay(1);

	/* Put also put Reset high ? */
}
