This directory includes source code for the "Phase 1" Google ADK plan.

The idea is to show a trivial example of an ADK application made up of an
Android device and a PC-based "Accessory".

The PC-based accessory searches on the USB bus for a Nexus One phone or
a Google Accessory and sets things up according to the ADK protocol.

Once the device has been succesfully configured, the ADKPing app should
automatically start and wait for some data on the USB port. The accessory
hence sends some data and the device will reply it back.


Accessory build and operating instructions accessory
-install libusb-1.0-0 and libusb-1.0-0-dev libraries. Note that old libusb
 wont work, as APIs are different
-Just enter accessory directory and make
-Run executable with sudo


Android application build and operating instructions
-Install Android IDE, make sure you have API level 10 in order to use accessory.
 Follow online documentation
-Download slf4j library for Android from www.slf4j.org/android and copy it
 somewhere in your SDK (e.g.: c:/android-sdk/add-ons)
-Create a new Android project with "File/New/Project/Android/Android Project"
 and select "Create new project from existing source" to pass the path of
 the application sources
-Add slf4j library using "Project/Properties/Java Build Path/Add External JARs"
-Build and run application using "Project/build Project"
-Launch the accessory code on PC and a notification message should pop up
 on the device asking permissions to handle the accessory. Say yes.



