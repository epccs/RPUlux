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

# for user to see
print("exampel for Digital firmware on RPUlux")
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

# set a DIO pin in OUTPUT mode
command = "/"+rpu_addr+"/pMod 14,OUTPUT"
sio.write((command  + "\n").encode('ascii')) 
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
json_reply = sio.readline().strip()
print("json echo: " + json_reply.decode("utf-8"))

# set a DIO pin  in INPUT mode
command = "/"+rpu_addr+"/pMod 15,INPUT"
sio.write((command  + "\n").encode('ascii')) 
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
json_reply = sio.readline().strip()
print("json echo: " + json_reply.decode("utf-8"))

# set the output HIGH and read the input
command = "/"+rpu_addr+"/dWrt 14,HIGH"
sio.write((command  + "\n").encode('ascii')) 
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
json_reply = sio.readline().strip()
print("json echo: " + json_reply.decode("utf-8"))
command = "/"+rpu_addr+"/dRe? 15"
sio.write((command  + "\n").encode('ascii')) 
rpubus_echo = sio.readline().strip()
print("cmd echo: " + rpubus_echo.decode("utf-8"))
json_reply = sio.readline().strip()
print("json echo: " + json_reply.decode("utf-8"))

# deserialize the JSON (e.g. {"PC1":"LOW"} ) line into a python object
data = json.loads(json_reply) # this will cause an error if JSON is not valid
print( "Digital 15 is on pin PC1 and it reads: "+ data["PC1"] )
