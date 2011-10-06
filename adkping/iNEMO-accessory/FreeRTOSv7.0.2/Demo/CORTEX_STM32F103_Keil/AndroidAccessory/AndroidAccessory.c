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


#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>

#include <string.h>
#include "inemoutil.h"


#define USB_ACCESSORY_VENDOR_ID         0x18D1
#define USB_ACCESSORY_PRODUCT_ID        0x2D00

#define USB_ACCESSORY_ADB_PRODUCT_ID    0x2D01
#define ACCESSORY_STRING_MANUFACTURER   0
#define ACCESSORY_STRING_MODEL          1
#define ACCESSORY_STRING_DESCRIPTION    2
#define ACCESSORY_STRING_VERSION        3
#define ACCESSORY_STRING_URI            4
#define ACCESSORY_STRING_SERIAL         5

#define ACCESSORY_GET_PROTOCOL          51
#define ACCESSORY_SEND_STRING           52
#define ACCESSORY_START                 53

struct AndroidAccessory androidAccessory;


/* Constructor */
void AndroidAccessory(const char *manufacturer,
                                   const char *model,
                                   const char *description,
                                   const char *version,
                                   const char *uri,
                                   const char *serial)
{
	androidAccessory.manufacturer = manufacturer;
	androidAccessory.model = model;
	androidAccessory.description = description;
	androidAccessory.version = version;
	androidAccessory.uri = uri;
	androidAccessory.serial = serial;

	/* Also construct max3421e chip and USB stack */
	max3421e();
	usbUSB();
	print("Android Accessory contructed [ok]\r\n");
}								   


byte androidAccessoryIsAccessoryDevice(USB_DEVICE_DESCRIPTOR *desc)
{
	return desc->idVendor == 0x18d1 && (desc->idProduct == 0x2D00 || desc->idProduct == 0x2D01);
}


void androidAccessoryPowerOn(void)
{
    max3421ePowerOn();
    delay(200);
}




s16 androidAccessoryGetProtocol(byte addr)
{
    s16 protocol = 0xaaaa;
    usbCtrlReq(addr, 0,
                USB_SETUP_DEVICE_TO_HOST |
                USB_SETUP_TYPE_VENDOR |
                USB_SETUP_RECIPIENT_DEVICE,
                ACCESSORY_GET_PROTOCOL, 0, 0, 0, 2, (char *)&protocol);
    return protocol;
}




void androidAccessorySendString(byte addr, int index, const char *str)
{
    usbCtrlReq(addr, 0,
                USB_SETUP_HOST_TO_DEVICE |
                USB_SETUP_TYPE_VENDOR |
                USB_SETUP_RECIPIENT_DEVICE,
                ACCESSORY_SEND_STRING, 0, 0, index,
                strlen((char*)str) + 1, (char *)str);
}


bool androidAccessorySwitchDevice(byte addr)
{
    int protocol = androidAccessoryGetProtocol(addr);

    if (protocol == 1) {
        print("device supports protcol 1\r\n");
    } else {
        print("could not read device protocol version\r\n");
        return false;
    }

    androidAccessorySendString(addr, ACCESSORY_STRING_MANUFACTURER, androidAccessory.manufacturer);
    androidAccessorySendString(addr, ACCESSORY_STRING_MODEL, androidAccessory.model);
    androidAccessorySendString(addr, ACCESSORY_STRING_DESCRIPTION, androidAccessory.description);
    androidAccessorySendString(addr, ACCESSORY_STRING_VERSION, androidAccessory.version);
    androidAccessorySendString(addr, ACCESSORY_STRING_URI, androidAccessory.uri);
    androidAccessorySendString(addr, ACCESSORY_STRING_SERIAL, androidAccessory.serial);

    usbCtrlReq(addr, 0,
                USB_SETUP_HOST_TO_DEVICE |
                USB_SETUP_TYPE_VENDOR |
                USB_SETUP_RECIPIENT_DEVICE,
                ACCESSORY_START, 0, 0, 0, 0, NULL);

    while (usbGetUsbTaskState() != USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE) {
        max3421eTask();
        usbTask();
    }

    return true;
}



// Finds the first bulk IN and bulk OUT endpoints
bool androidAccessoryFindEndpoints(byte addr, EP_RECORD *inEp, EP_RECORD *outEp)
{
    u16 len;
    byte err;
    u8 *p;

    err = usbGetConfDescr(addr, 0, 4, 0, (char *)androidAccessory.descBuff);
    if (err) {
        print("Can't get config descriptor length\r\n");
        return false;
    }


    len = androidAccessory.descBuff[2] | ((u16)androidAccessory.descBuff[3] << 8);
    if (len > sizeof(androidAccessory.descBuff)) {
        print("config descriptor too large\r\n");
            /* might want to truncate here */
        return false;
    }

    err = usbGetConfDescr(addr, 0, len, 0, (char *)androidAccessory.descBuff);
    if (err) {
        print("Can't get config descriptor\r\n");
        return false;
    }

    p = androidAccessory.descBuff;
    inEp->epAddr = 0;
    outEp->epAddr = 0;
    while (p < (androidAccessory.descBuff + len)){
        u8 descLen = p[0];
        u8 descType = p[1];
        USB_ENDPOINT_DESCRIPTOR *epDesc;
        EP_RECORD *ep;

        switch (descType) {
        case USB_DESCRIPTOR_CONFIGURATION:
            print("config desc\r\n");
            break;

        case USB_DESCRIPTOR_INTERFACE:
            print("interface desc\r\n");
            break;

        case USB_DESCRIPTOR_ENDPOINT:
            epDesc = (USB_ENDPOINT_DESCRIPTOR *)p;
            if (!inEp->epAddr && (epDesc->bEndpointAddress & 0x80))
                ep = inEp;
            else if (!outEp->epAddr)
                ep = outEp;
            else
                ep = NULL;

            if (ep) {
                ep->epAddr = epDesc->bEndpointAddress & 0x7f;
                ep->Attr = epDesc->bmAttributes;
                ep->MaxPktSize = epDesc->wMaxPacketSize;
                ep->sndToggle = bmSNDTOG0;
                ep->rcvToggle = bmRCVTOG0;
            }
            break;

        default:
            print("unkown desc type \r\n");
            //println( descType, HEX);
            break;
        }

        p += descLen;
    }

    if (!(inEp->epAddr && outEp->epAddr))
        print("can't find accessory endpoints\r\n");

    return inEp->epAddr && outEp->epAddr;
}


bool androidAccessoryConfigureAndroid(void)
{
    byte err;
    EP_RECORD inEp, outEp;
	
    if (!androidAccessoryFindEndpoints(1, &inEp, &outEp))
        return false;

    memset(&androidAccessory.epRecord, 0x0, sizeof(androidAccessory.epRecord));

    androidAccessory.epRecord[inEp.epAddr] = inEp;
    if (outEp.epAddr != inEp.epAddr)
        androidAccessory.epRecord[outEp.epAddr] = outEp;

    androidAccessory.in = inEp.epAddr;
    androidAccessory.out = outEp.epAddr;

	print("End points: ");
    printHex(inEp.epAddr);
	print("  ");
    printHex(outEp.epAddr);
	print("\r\n");

    androidAccessory.epRecord[0] = *(usbGetDevTableEntry(0,0));
    usbSetDevTableEntry(1, androidAccessory.epRecord);

    err = usbSetConf( 1, 0, 1 );
    if (err) {
        print("Can't set config to 1\r\n");
        return false;
    }

    usbSetUsbTaskState( USB_STATE_RUNNING );

    return true;
}

/* Start from here...*/

bool androidAccessoryIsConnected(void)
{
    USB_DEVICE_DESCRIPTOR *devDesc = (USB_DEVICE_DESCRIPTOR *) androidAccessory.descBuff;
    byte err;

    max3421eTask();
    usbTask();

    if (!androidAccessory.connected &&
        usbGetUsbTaskState() >= USB_STATE_CONFIGURING &&
        usbGetUsbTaskState() != USB_STATE_RUNNING) {
        print("nDevice addressed... \r\n");
        print("Requesting device descriptor.\r\n");

        err = usbGetDevDescr(1, 0, 0x12, (char *) devDesc);
        if (err) {
            print("Device descriptor cannot be retrieved. Trying again\r\n");
            return false;
        }

        if (androidAccessoryIsAccessoryDevice(devDesc)) {
            print("found android acessory device\r\n");

            androidAccessory.connected = androidAccessoryConfigureAndroid();
        } else {
            print("found possible device. swithcing to serial mode\r\n");
            androidAccessorySwitchDevice(1);
        }
    } else if (usbGetUsbTaskState() == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE) {
        if (androidAccessory.connected)
            print("disconnect\r\n");
        androidAccessory.connected = false;
    }

    return androidAccessory.connected;
}


int androidAccessoryRead(void *buff, int len, unsigned int nakLimit)
{
    return usbNewInTransfer(1, androidAccessory.in, len, (char *)buff);
}

int androidAccessoryWrite(void *buff, int len)
{
    usbOutTransfer(1, androidAccessory.out, len, (char *)buff);
    return len;
}

