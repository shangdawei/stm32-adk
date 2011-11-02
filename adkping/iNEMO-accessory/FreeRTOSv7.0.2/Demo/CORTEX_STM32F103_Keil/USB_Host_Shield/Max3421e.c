/*
 * Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com
 * MAX3421E USB host controller support
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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
#include "Max3421e_constants.h"

#include "inemoutil.h"
#include "stm32_types_compat.h"

static u8 vbusState;

/* 
 * Public methods
 */


void max3421e(void)
{
	pinInit();
	spi_init();
}


void max3421eRegWr(u8 reg, u8 val)
{
	/* Slave select low
	 * The software SS 	SPI_SSOutputCmd(SPI1, ENABLE);
	 * is not working properly, relying on GPIO instead */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET);
	
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
}			 


/* Cfr:  LCD_ReadReg */
u8 max3421eRegRd(u8 reg)
{
	u8 rddata;

	/* Slave select low. According to Maxim spec (p13), we should wait at
	 * least tl=30ns */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET);
	
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
			 
	return rddata;
}

char* max3421eBytesWr(u8 reg, u8 nbytes, char* data )
{
	while(nbytes--){
		max3421eRegWr(reg, *data);
		data++;
	}
    return( data );
}


char* max3421eBytesRd(u8 reg, u8 nbytes, char* data)
{
	while(nbytes--){
		*data = max3421eRegRd(reg);
		data++;
	}

	return data;
}


u8 max3421eReset(void)
{
	u16 tmp = 0;
    max3421eRegWr( rUSBCTL, bmCHIPRES );                        //Chip reset. This stops the oscillator
    max3421eRegWr( rUSBCTL, 0x00 );                             //Remove the reset

    while(!(max3421eRegRd( rUSBIRQ ) & bmOSCOKIRQ )) {  //wait until the PLL is stable
		vTaskDelay(10);
        tmp++;                                          //timeout after 100ms
        if( tmp == 10 ) {
            return( false );
        }
    }
    return( true );
}


void max3421ePowerOn(void)
{
	max3421eRegWr( rPINCTL,( bmFDUPSPI + bmINTLEVEL + bmGPXB ));	// Full-duplex SPI, level interrupt, GPX
	if( max3421eReset() == false ) {                                // stop/start the oscillator
        print("Error: OSCOKIRQ failed to assert\r\n");
		panic();
    }else{
		print("MAX3421e reset [ok]\r\n");
	}


    /* configure host operation */
    max3421eRegWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ );      // set pull-downs, Host, Separate GPIN IRQ on GPX
    max3421eRegWr( rHIEN, bmCONDETIE|bmFRAMEIE );                       // connection detection
    /* check if device is connected */
    max3421eRegWr( rHCTL,bmSAMPLEBUS );                                 // sample USB bus
    while(!(max3421eRegRd( rHCTL ) & bmSAMPLEBUS ));                    // wait for sample operation to finish
    busprobe();                                                       // check if anything is connected
    max3421eRegWr( rHIRQ, bmCONDETIRQ );                                // clear connection detect interrupt                 
    max3421eRegWr( rCPUCTL, 0x01 );                                     // enable interrupt pin
	
	print("MAX3421e powered on [ok]\r\n");	
}


/* MAX3421 state change task and interrupt handler */
u8 max3421eTask( void )
{
 	u8 rcode = 0;
 	u8 pinvalue;
    //Serial.print("Vbus state: ");
    //Serial.println( vbusState, HEX );
 	pinvalue = readINT();
 	if( pinvalue  == Bit_RESET ) {
        rcode = IntHandler();
    }
    pinvalue = readGPX();
    if( pinvalue == Bit_RESET ) {
        GpxHandler();
    }
//    usbSM();                                //USB state machine                            
    return( rcode );   
}


/* Private methods
 */

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
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  	SPI_Init(SPI1, &SPI_InitStructure);

  	/* SPI1 enable */
  	SPI_Cmd(SPI1, ENABLE);	
}


void pinInit(void)
{
	/* Slave select high */
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET);

	/* Also Reset line MUST be se high otherwise reset() wont work 
	 * In our setup it is wired to 3.3V
	 */
}


/* probe bus to determine device presense and speed and switch host to this speed */
void busprobe(void)
{
	u8 bus_sample;
    bus_sample = max3421eRegRd( rHRSL );            //Get J,K status
    bus_sample &= ( bmJSTATUS|bmKSTATUS );      //zero the rest of the byte
    switch( bus_sample ) {                          //start full-speed or low-speed host 
        case( bmJSTATUS ):
            if(( max3421eRegRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                max3421eRegWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            else {
                max3421eRegWr( rMODE, MODE_LS_HOST);        //start low-speed host
                vbusState = LSHOST;
            }
            break;
        case( bmKSTATUS ):
            if(( max3421eRegRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                max3421eRegWr( rMODE, MODE_LS_HOST );       //start low-speed host
                vbusState = LSHOST;
            }
            else {
                max3421eRegWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            break;
        case( bmSE1 ):              //illegal state
            vbusState = SE1;
            break;
        case( bmSE0 ):              //disconnected state
		max3421eRegWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ);
            vbusState = SE0;
            break;
        }//end switch( bus_sample )
}


u8 getVbusState(void)
{ 
    return( vbusState );
}


u8 IntHandler(void)
{
 	u8 HIRQ;
 	u8 HIRQ_sendback = 0x00;
    HIRQ = max3421eRegRd( rHIRQ );                  //determine interrupt source
    //if( HIRQ & bmFRAMEIRQ ) {               //->1ms SOF interrupt handler
    //    HIRQ_sendback |= bmFRAMEIRQ;
    //}//end FRAMEIRQ handling
    if( HIRQ & bmCONDETIRQ ) {
        busprobe();
        HIRQ_sendback |= bmCONDETIRQ;
    }
    /* End HIRQ interrupts handling, clear serviced IRQs    */
    max3421eRegWr( rHIRQ, HIRQ_sendback );
    return( HIRQ_sendback );
}


u8 GpxHandler(void)
{
	u8 GPINIRQ = max3421eRegRd( rGPINIRQ );          //read GPIN IRQ register
//    if( GPINIRQ & bmGPINIRQ7 ) {            //vbus overload
//        vbusPwr( OFF );                     //attempt powercycle
//        delay( 1000 );
//        vbusPwr( ON );
//        regWr( rGPINIRQ, bmGPINIRQ7 );
//    }       
    return( GPINIRQ );
}


u8 readINT(void)
{
	return (u8)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7);
}

u8 readGPX(void)
{
	// return GPX_PIN & _BV(GPX) ? HIGH : LOW;
	return (u8)Bit_RESET;
}


