#!/usr/bin/env python3
# Line ending: on Linux lines end with LF, on Windows most editors add a CR-LF pair 
# but that will confuse bash and/or the env function and thus most liky look for "python\r" and fail. 
# this python file also needs to be set as an executable with chmod +x load_profile.py

# I am using python3 so make sure the package is installed for it. 
# sudo apt-get update
# sudo apt-get install python3-serial 
import json
import serial
from time import sleep

# address on the RPU_BUS to interact with
rpu_addr = '1'

# When the host serial port is open it will cause nDTR or nRTS to be set active wHIGH will tell the
# RPUadpt to send a bootload address and  
# reset a device that will run a bootloader for a few seconds (set the timeout longer if the bootloader needs more time).
sio = serial.Serial("/dev/ttyUSB0",38400, timeout=3)

rpubus_echo = sio.readline().strip()

# status for user to see
print("exampel for Pwm firmware on RPUlux")
print("init: " + rpubus_echo[-10:].decode("utf-8")) 

# if a command is running somewhere after a reset stop it
sio.write(("\n").encode('ascii'))
sleep(0.2)
command = "/"+rpu_addr
sio.write((command + "\n").encode('ascii'))
rpubus_echo = sio.readline().strip()
print("echo: " + rpubus_echo.decode("utf-8"))
while (not (rpubus_echo == (command).encode('ascii') ) ) :
    sleep(0.2)
    print("miss catch of command prompt, trying again")
    sio.write((command + "\n").encode('ascii'))
    rpubus_echo = sio.readline().strip()
    print("echo: " + rpubus_echo.decode("utf-8"))
print("ready for command")

# each level doubles the amount of light output
LumSteps = [0,1,3,7,15,31,63,127,255]
LedChannels = ["CH1","CH2","CH3","CH4","CH5","CH6"]

# Ramp up each LED then ramp down
for Ch in LedChannels:
    for Step in LumSteps:
        command = "/"+rpu_addr+"/pwm " + Ch + "," + str(Step)
        sio.write((command  + "\n").encode('ascii')) 
        rpubus_echo = sio.readline().strip()
        print("cmd echo: " + rpubus_echo.decode("utf-8"))
        rpubus_echo = sio.readline().strip()
        print("json echo: " + rpubus_echo.decode("utf-8"))
        sleep(0.2)
    for Step in LumSteps[::-1]: # reverse
        command = "/"+rpu_addr+"/pwm " + Ch + "," + str(Step)
        sio.write((command  + "\n").encode('ascii')) 
        rpubus_echo = sio.readline().strip()
        print("cmd echo: " + rpubus_echo.decode("utf-8"))
        rpubus_echo = sio.readline().strip()
        print("json echo: " + rpubus_echo.decode("utf-8"))
        sleep(0.2)
