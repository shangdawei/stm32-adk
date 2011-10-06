/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __AndroidAccessory_h__
#define __AndroidAccessory_h__

#include <Usb.h>

#define bool u8
#define byte u8

struct AndroidAccessory{
    const char *manufacturer;
    const char *model;
    const char *description;
    const char *version;
    const char *uri;
    const char *serial;

    //MAX3421E max;
    //USB usb;
    bool connected;
    u8 in;
    u8 out;

    EP_RECORD epRecord[8];
    u8 descBuff[256];
};

byte androidAccessoryIsAccessoryDevice(USB_DEVICE_DESCRIPTOR *desc);

s16 androidAccessoryGetProtocol(byte addr);
void androidAccessorySendString(byte addr, int index, const char *str);
bool androidAccessorySwitchDevice(byte addr);
bool androidAccessoryFindEndpoints(byte addr, EP_RECORD *inEp, EP_RECORD *outEp);
bool androidAccessoryConfigureAndroid(void);


void AndroidAccessory(const char *manufacturer,
                 const char *model,
                 const char *description,
                 const char *version,
                 const char *uri,
                 const char *serial);

void androidAccessoryPowerOn(void);

bool androidAccessoryIsConnected(void);
int androidAccessoryRead(void *buff, int len, unsigned int nakLimit);
int androidAccessoryWrite(void *buff, int len);


#endif /* __AndroidAccessory_h__ */
