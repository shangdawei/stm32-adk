/**	iNEMO utility functions
 */
#ifndef __INEMO_UTIL_H
#define __INEMO_UTIL_H

#define BOARD_IS_INEMOV2  1

/* Task priorities. */

#define IN_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )


/* The check task uses the sprintf function so requires a little more stack. */

#define IN_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 200)

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

#endif
