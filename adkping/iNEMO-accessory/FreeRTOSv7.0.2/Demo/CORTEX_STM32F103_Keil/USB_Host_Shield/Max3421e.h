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

#include "FreeRTOS.h"
#include "stm32f10x_lib.h"

#include "Max3421e_constants.h"

#define false (0)
#define true  (~0)


/* Public methods */
void max3421e(void); //constructor
void max3421eRegWr(u8 reg, u8 val);
u8 max3421eRegRd(u8 reg);
char* max3421eBytesWr(u8 reg, u8 nbytes, char* data);  // TO BE TESTED!
char* max3421eBytesRd(u8 reg, u8 nbytes, char* data); // TO BE TESTED!
void max3421ePowerOn(void);
u8 max3421eReset(void);
u8 max3421eTask(void);

/* Prvivate methods (as from Max3421e.cpp Arduino code) */
void spi_init(void);
void pinInit(void);
void busprobe(void);
u8 getVbusState(void);
u8 IntHandler(void);
u8 GpxHandler(void);
u8 GpxHandler(void);
u8 readINT(void);
u8 readGPX(void);
