This directory includes source code for the "Phase 1" Google ADK plan.

The idea is to show a trivial example of an ADK application made up of an
Android device and a PC-based "Accessory".

The PC-based accessory searches on the USB bus for a Nexus One phone or
a Google Accessory and sets things up according to the ADK protocol.

Once the device has been succesfully configured, the ADKPing app should
automatically start and wait for some data on the USB port. The accessory
hence sends some data and the device will reply it back.

