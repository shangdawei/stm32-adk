/**	iNEMO utility functions
 */

#define BOARD_IS_INEMOV2  1

void prvSetupHardware(void);
void gpiosInit(void);

int inemoUtilInit(void);
void ledOn(void);
void ledOff(void);
int print(char* string);
void printHex(unsigned int);
unsigned int millis(void);
void delay(int delay);
void panic(void);

