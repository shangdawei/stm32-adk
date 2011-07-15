/*
 * \file adkping.c
 */


#include <stdio.h>
#include <usb.h>
#include <libusb.h>
#include <string.h>
#include <unistd.h>


/* Nexus-One IDs */
#define VID 0x18D1
#define PID 0x4E12

/* Google Accessory IDs */
#define ACCESSORY_VID 0x18D1
#define ACCESSORY_PID 0x2D00
#define ACCESSORY_PID_ALT 0x2D01


/* Nexus-One Accessory device end points */
#define IN  0x83
#define OUT 0x03

#define LEN 2

unsigned char buffer[1024];

static int mainPhase();
static int deInit(void);
static void error(int code);
static void status(int code);
static int setupAccessory(
	char* manufacturer,
	char* modelName,
	char* description,
	char* version,
	char* uri,
	char* serialNumber);

static struct libusb_device_handle* handle;

int main(int argc, char *argv[])
{
	int err;
	libusb_init(NULL);

	/* Try to open directly accessory. If it's not there, try to switch phone to it */
	handle = libusb_open_device_with_vid_pid(NULL, ACCESSORY_VID, ACCESSORY_PID_ALT);
	if(handle == NULL){
		printf("Device not in Accessory mode. Trying to switch it to it...\n");
		err = setupAccessory("STMicroelectronics", "adkping", "Just pings data", "1.0",
			"http://www.st.com", "1234567890123456");
		if(err < 0){
			fprintf(stdout, "Sorry, can't set up accessory, giving up\n");
			deInit();
			return -1;
		}
	}

	/* Do some stuff */
	if(mainPhase() < 0){
		fprintf(stdout, "Error during main phase\n");
		deInit();
		return -1;
	}
	
	deInit();
	fprintf(stdout, "Done, no errors\n");

	return 0;
}


static int mainPhase(){
	int response = 0;
	int i;
	static int transferred;

#if 0
	struct libusb_config_descriptor* config_desc;
	const struct libusb_interface_descriptor* interface_desc;
	
	response = libusb_get_descriptor(handle, LIBUSB_DT_CONFIG, 0, buffer, LIBUSB_DT_CONFIG_SIZE);
	//response = libusb_get_descriptor(handle, LIBUSB_DT_INTERFACE, 0, buffer, LIBUSB_DT_INTERFACE_SIZE);
	if(response < 0){error(response);return -1;}

	config_desc = (struct libusb_config_descriptor*)buffer;
	printf("bLength %i\n", config_desc->bLength);
	printf("bNumInterfaces %i\n", config_desc->bNumInterfaces);
	printf("MaxPower %i\n", config_desc->MaxPower);
#endif
	
	/* Send something */
	memset(buffer, 0xdd, sizeof(buffer));
	for(i=0; i<128; i++){
		buffer[i] = 'a' + i;
	}
	response = libusb_bulk_transfer(handle, OUT, buffer, 64, &transferred, 0);
	if(response < 0){
		error(response);
		return -1;
	}
	else{
		status(response);
	}
	
	printf("Done, transferred %i bytes\n", transferred);

	return 0;
}


static int deInit(){
	//TODO free all transfers individually...
	//if(ctrlTransfer != NULL)
	//	libusb_free_transfer(ctrlTransfer);
	if(handle != NULL)
		libusb_release_interface (handle, 0);
	libusb_exit(NULL);
	return 0;
}


int connectAccessory(void)
{
	int response;
	int tries = 5;
	
	/* Try connecting to Accessory device with proper PID/VID */
	for(;;){
		tries--;
		if((handle = libusb_open_device_with_vid_pid(NULL, ACCESSORY_VID, ACCESSORY_PID_ALT)) == NULL){
			if(tries < 0){
				return -1;
			}
		}else{
			break;
		}
		sleep(1);
	}

	/* Set configuration to 1 as per ADK protocol */
	response = libusb_set_configuration(handle, 1);
	if(response < 0){
		error(response);
		return -1;
	}
	
	response = libusb_claim_interface(handle, 0);
	if(response < 0){
		error(response);
		return -1;
	}

	fprintf(stdout, "Interface claimed, ready to transfer data\n");

	return 0;
}


/**
 * Attempts to put USB device in Accessory mode.
 * On success, PID/VID switches to accessory PID/VID
 * and proper application is launched. Application must match
 * Accessory manufacturer, model and version which are defined in
 * "android.hardware.us.action.USB_ACCESSORY_ATTACHED" meta-data
 */
static int setupAccessory(
	char* manufacturer,
	char* modelName,
	char* description,
	char* version,
	char* uri,
	char* serialNumber){
	
	unsigned char ioBuffer[2];
	int devVersion;
	int response;

	/* Open Nexus One device */
	if((handle = libusb_open_device_with_vid_pid(NULL, VID, PID)) == NULL){
		fprintf(stdout, "Problem acquireing handle\n");
		return -1;
	}
	libusb_claim_interface(handle, 0);

	
	/* Send GET_PROTOCOL (0x51) request to figure out whether the device supports ADK
	 * If protocol is  not 0 the device supports it
	 */
	response = libusb_control_transfer(
		handle, //handle
		0xC0, //bmRequestType
		51, //bRequest
		0, //wValue
		0, //wIndex
		ioBuffer, //data
		2, //wLength
		0 //timeout
	);

	if(response < 0){
		error(response);
		return -1;
	}

	/* Extract protocol version to check whether Accessory mode is supported */
	devVersion = ioBuffer[1] << 8 | ioBuffer[0];
	fprintf(stdout,"Version Code Device: %d\n", devVersion);

	if(devVersion == 0){
		fprintf(stdout,"Sorry, device is not supporting Accessory mode\n");
		return -1;
	}
	
	usleep(1000);//sometimes hangs on the next transfer :(

	/* Accessory mode is supported: send out IDs */
	response = libusb_control_transfer(handle,0x40,52,0,0,(unsigned char*)manufacturer,strlen(manufacturer),0);
	if(response < 0){error(response);return -1;}

	response = libusb_control_transfer(handle,0x40,52,0,1,(unsigned char*)modelName,strlen(modelName)+1,0);
	if(response < 0){error(response);return -1;}

	response = libusb_control_transfer(handle,0x40,52,0,2,(unsigned char*)description,strlen(description)+1,0);
	if(response < 0){error(response);return -1;}

	response = libusb_control_transfer(handle,0x40,52,0,3,(unsigned char*)version,strlen(version)+1,0);
	if(response < 0){error(response);return -1;}

	response = libusb_control_transfer(handle,0x40,52,0,4,(unsigned char*)uri,strlen(uri)+1,0);
	if(response < 0){error(response);return -1;}

	response = libusb_control_transfer(handle,0x40,52,0,5,(unsigned char*)serialNumber,strlen(serialNumber)+1,0);
	if(response < 0){error(response);return -1;}

	fprintf(stdout,"Accessory Identification sent\n");

	/* Request device to start up in accessory mode */
	response = libusb_control_transfer(handle,0x40,53,0,0,NULL,0,0);
	if(response < 0){
		error(response);
		return -1;
	}

	fprintf(stdout,"Attempted to put device into accessory mode\n");

	/* Close phone handles as we need to connect to Accessory device in a while */
	if(handle != NULL){
		libusb_release_interface(handle, 0);
		libusb_close(handle);
	}

	response = connectAccessory();
	if(response < 0){
		fprintf(stderr, "Could not connect to Accessory device\n");
		return -1;
	}
	
	return 0;
}

static void error(int code){
	fprintf(stdout,"\n");
	switch(code){
	case LIBUSB_ERROR_IO:
		fprintf(stdout,"Error: LIBUSB_ERROR_IO\nInput/output error.\n");
		break;
	case LIBUSB_ERROR_INVALID_PARAM:
		fprintf(stdout,"Error: LIBUSB_ERROR_INVALID_PARAM\nInvalid parameter.\n");
		break;
	case LIBUSB_ERROR_ACCESS:
		fprintf(stdout,"Error: LIBUSB_ERROR_ACCESS\nAccess denied (insufficient permissions).\n");
		break;
	case LIBUSB_ERROR_NO_DEVICE:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_DEVICE\nNo such device (it may have been disconnected).\n");
		break;
	case LIBUSB_ERROR_NOT_FOUND:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_FOUND\nEntity not found.\n");
		break;
	case LIBUSB_ERROR_BUSY:
		fprintf(stdout,"Error: LIBUSB_ERROR_BUSY\nResource busy.\n");
		break;
	case LIBUSB_ERROR_TIMEOUT:
		fprintf(stdout,"Error: LIBUSB_ERROR_TIMEOUT\nOperation timed out.\n");
		break;
	case LIBUSB_ERROR_OVERFLOW:
		fprintf(stdout,"Error: LIBUSB_ERROR_OVERFLOW\nOverflow.\n");
		break;
	case LIBUSB_ERROR_PIPE:
		fprintf(stdout,"Error: LIBUSB_ERROR_PIPE\nPipe error.\n");
		break;
	case LIBUSB_ERROR_INTERRUPTED:
		fprintf(stdout,"Error:LIBUSB_ERROR_INTERRUPTED\nSystem call interrupted (perhaps due to signal).\n");
		break;
	case LIBUSB_ERROR_NO_MEM:
		fprintf(stdout,"Error: LIBUSB_ERROR_NO_MEM\nInsufficient memory.\n");
		break;
	case LIBUSB_ERROR_NOT_SUPPORTED:
		fprintf(stdout,"Error: LIBUSB_ERROR_NOT_SUPPORTED\nOperation not supported or unimplemented on this platform.\n");
		break;
	case LIBUSB_ERROR_OTHER:
		fprintf(stdout,"Error: LIBUSB_ERROR_OTHER\nOther error.\n");
		break;
	default:
		fprintf(stdout, "Error: unkown error\n");
	}
}


static void status(int code){
	fprintf(stdout,"\n");
	switch(code){
		case LIBUSB_TRANSFER_COMPLETED:
			fprintf(stdout,"Success: LIBUSB_TRANSFER_COMPLETED\nTransfer completed.\n");
			break;
		case LIBUSB_TRANSFER_ERROR:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_ERROR\nTransfer failed.\n");
			break;
		case LIBUSB_TRANSFER_TIMED_OUT:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_TIMED_OUT\nTransfer timed out.\n");
			break;
		case LIBUSB_TRANSFER_CANCELLED:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_CANCELLED\nTransfer was cancelled.\n");
			break;
		case LIBUSB_TRANSFER_STALL:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_STALL\nFor bulk/interrupt endpoints: halt condition detected (endpoint stalled).\nFor control endpoints: control request not supported.\n");
			break;
		case LIBUSB_TRANSFER_NO_DEVICE:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_NO_DEVICE\nDevice was disconnected.\n");
			break;
		case LIBUSB_TRANSFER_OVERFLOW:
			fprintf(stdout,"Error: LIBUSB_TRANSFER_OVERFLOW\nDevice sent more data than requested.\n");
			break;
		default:
			fprintf(stdout,"Error: unknown error\nTry again(?)\n");
			break;
	}
}
