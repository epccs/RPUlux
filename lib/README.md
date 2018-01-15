# Common Files

## Overview

These files are used by Applications that have a Makefile in their folder, this folder does not have a Makefile. These files make up the library the can be used to control the microcontroller peripherals. It is at best incomplete. 

While I was trying to gather the bits and pieces I did not put enough time into trying to track who did what, I made sure it was an open license and then overhauled and fashioned to suit my taste. I have tried to give credit where it made since to do so. 

I think credit needs to be given for [Wiring]<https://arduinohistory.github.io/> which is where many of the ideas seem to originate for commands like digitalRead(), digitalWrite(). 

# Notes

The toolchain for this physical computing device can live on a single board computer (SBC's) running Linux that is attached to one or more of the devices. This guarantees that the physical computing firmware can be compiled and updated rather or not a network connection can be established. An intermittent network connection can still be used to update the toolchain and services. The Linux SBC is not a good replacement for a bare metal physical computing device, partly because an MCU can boot and be in control of equipment almost instantly, and some control needs are not a good fit for a timeshare system like Linux.