# Collision-detector-and-GPS-locator

What this project does is that it monitors your angle with respect to earth and see for some unusual angle. 
Like for example when car is flipped or jump in a crash. In case when this is detected,it asks the driver if he is ok by pressing a button.
If the button is not pressed, it means the driver is injured or unconcious and it sends an SOS message alongwith the location of the driver to the registered phone number.

This function can be turned on or off too by sending specific messages to the sim number of module.There are options to read messages from all sims or soe specific sims.
Also in case of theft or security purposes, its location can be tracked remotely by sending a specific message. It only sends location to the registored number(s). Currently it only has support for one number but it can be extended easily.

The programming is for standalone AVR microconroller ATmega256 at 16MHz. Code is not dependent on arduino and not uses its functions. But it does uses some basic libraries like math.h,avr/io.h and string.h.
