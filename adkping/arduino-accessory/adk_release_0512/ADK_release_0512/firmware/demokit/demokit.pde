#include <Max3421e.h>
#include <Usb.h>
#include <AndroidAccessory.h>


AndroidAccessory acc("STMicroelectronics", "adkping", "Just pings data", "2.0",
                        "http://www.st.com", "1234567890123456");

char sendBuffer[]="ArduinoAccessory!";
char receiveBuffer[128];
int len;    
void setup();
void loop();
void mainPhase();    



void setup()
{
	Serial.begin(115200);
	Serial.print("\r\nStart");
	acc.powerOn();
}

void loop()
{


        if (acc.isConnected()) {
                Serial.print("\r\nInside acc.isconnected, calling mainphase...");
                mainPhase();
                Serial.print("\r\nExited from mainphase...");
        }
        Serial.print("\r\nTest loop...");
	delay(10);
}

void mainPhase(){
  while(1){    
        len=acc.write(sendBuffer,17);
        Serial.print("\r\nSent accessory name... Total chars sent:");
        Serial.print(len);
        Serial.print("\r\nReceived from accessory: ");
        len = acc.read(receiveBuffer, sizeof(receiveBuffer), 1);
        Serial.print(receiveBuffer);
        Serial.print(" Total chars received: ");
        Serial.print(len);
        delay(1000);
  }      
}
