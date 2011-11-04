This directory contains source code for ADKPing demo application.

ADKPing is a very simple Android application and related accessory firmware
that demonstrates Google's ADK functionalities:

-ADK accessory handshakes with Android application via ADK protocol
-ADK accessory sends a string to the Android device
-Android device receives string thru ADK, displays it, concatenates another
 string, and sends it back to the accessory
-Accessory shows concatenated string on console (or serial line for
 microcontroller-based accessories)

Contents:
-ADKPing: Android application
-iNEMO-accessory: STMicroelectronics iNEMOv2 based accessory
-arduino-accessory: official Google accessory. Runs on Arduino Uno with
 SparkFun USB host shield
-pc-accessory: PC based accessory


