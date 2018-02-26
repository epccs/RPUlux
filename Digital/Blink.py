#!/usr/bin/env python3
# Line ending is the gift from hell, on Linux lines end with LF, on Windows most editors add a CR-LF pair 
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
print("init: " + rpubus_echo[-10:].decode("utf-8")) 

# checking if an output ? command was printed 
if ( rpubus_echo[-7:] == b'?'):
    # stop the command that may be running after a reset
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
    print("ready for next command")
else:
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
    print("ready for next command")

# set the the LED control pins in OUTPUT mode
command = "/"+rpu_addr+"/pMod 3,OUTPUT"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
command = "/"+rpu_addr+"/pMod 5,OUTPUT"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
command = "/"+rpu_addr+"/pMod 6,OUTPUT"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
command = "/"+rpu_addr+"/pMod 9,OUTPUT"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
command = "/"+rpu_addr+"/pMod 10,OUTPUT"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
command = "/"+rpu_addr+"/pMod 11,OUTPUT"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))

# Turn on each LED for a few seconds
command = "/"+rpu_addr+"/dWrt 3,HIGH"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
sleep(5)
command = "/"+rpu_addr+"/dWrt 3,LOW"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))


command = "/"+rpu_addr+"/dWrt 5,HIGH"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
sleep(5)
command = "/"+rpu_addr+"/dWrt 5,LOW"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))

command = "/"+rpu_addr+"/dWrt 6,HIGH"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
sleep(5)
command = "/"+rpu_addr+"/dWrt 6,LOW"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))

command = "/"+rpu_addr+"/dWrt 9,HIGH"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
sleep(5)
command = "/"+rpu_addr+"/dWrt 9,LOW"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))

command = "/"+rpu_addr+"/dWrt 10,HIGH"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
sleep(5)
command = "/"+rpu_addr+"/dWrt 10,LOW"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))

command = "/"+rpu_addr+"/dWrt 11,HIGH"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
sleep(5)
command = "/"+rpu_addr+"/dWrt 11,LOW"
sio.write((command  + "\n").encode('ascii')) 
sleep(0.2)
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
rpubus_echo = sio.readline().strip()
print("json echo: " + rpubus_echo.decode("utf-8"))
