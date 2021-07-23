'''
List of PIDs and Service Numbers here: https://en.wikipedia.org/wiki/OBD-II_PIDs
https://ultra-gauge.com/ultragauge/Design_Files/images/Windshield_Mount_Installed.jpg
https://www.windmill.co.uk/obdii.pdf
'''

import serial
from time import sleep

ser = serial.Serial(port='COM3', baudrate=115200, timeout=1)

# Raw Hex interfacing with ECU
#          +- Service Number
#          |  +- PID
#          V  V 
ser.write("01 0D \r") # Speed in kph
speed_hex = ser.readline().split(' ')
speed = float(int('0x'+speed_hex[3], 0)) # Convert hex to decimal
print(f"Speed: {speed} km/h")

"""
Update: Figured it out.

"Module/Header" set to "ECM"
"OBD Mode" set to "21"
"PID Number" set to "01"
and to output Fahrenheit, "Equation" set to "((((AC-40)*9)/5)+32)" 

Wiki says equation is A-40
"""
while(1):
   ser.write("01 5C \r") # Engine oil temperature
   oil_hex = ser.readline().split(' ')
   oil_temp = int('0x'+oil_hex[3], 0) - 40
   print(f"Returned hex {oil_hex}")
   print(f"Oil Temp: {oil_temp} C   {(oil_temp*1.8)+32} F")
   sleep(0.5)