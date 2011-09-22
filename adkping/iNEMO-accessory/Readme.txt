This folder holds code for the iNEMO ADK accessory project. The objective of
this work is to come up with an iNEMO based Android ADK accessory in place of
the canonical Arduino based one.

Arduino accessory firmware code is roughly architected as follows:


demokit.pde --> ADK Accessory library ----> Maxim library
                              |                    ^
                              |                    |
                              |                    |
                              ------------> USB shield library


Plan
-Code micro delay functions and test them on SPI read/write code. Needed for
 Slave Select signal timings
-Test hardware Slave Select to save one GPIO pin in iNEMO
-Enable/test UART2 for debuggin purposes [ OK ]
-Fix pinout iNEMO/USB Shield
-Start from latest, clena FreeRTOS and put everything on Git [ OK ]
-Study and port Arduino firmware



                

