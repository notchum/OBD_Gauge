# OBD_Gauge
A simple Arduino-powered automotive gauge to display readings from various car sensors.
The readings that are currently implemented are:
- Air/Fuel Ratio (AFR)
- Battery Voltage
- Oil Temperature (in F)
- Coolant Temperature (in F)
- Air Intake Temperature (in F)

This project was developed for and tested on a 2013 Scion FR-S, other vehicles may have different sensor setups and ECU calculations for readings.

# DEPENDENCIES
- [ArduinoOBD](https://github.com/stanleyhuangyc/ArduinoOBD)


# HARDWARE
This project uses an Arduino Nano, a 128*64 I2C OLED, and a [Freematics OBD-II UART Adapter](https://freematics.com/products).

> Pin A0 -> Button -> 10k resistor -> GND \
> Pin A4 -> SDA on OLED (10k pullup resistor) \
> Pin A5 -> SCL on OLED (10k pullup resistor) \
> Pin TX1 -> RX on OBD-II UART Adapter \
> Pin RX0 -> TX on OBD-II UART Adapter \
> 5V and GND is provided from OBD-II UART Adapter for Arduino and OLED

# REFERENCES
- A user named robot on the FT86CLUB forums inspired this project with his [post](https://www.ft86club.com/forums/showthread.php?t=75181&highlight=arduino) and his [code](https://github.com/stirobot/arduinoModularTFTgauges/blob/master/oledOBDgaugesSmallIrvinedLib/oledOBDgaugesSmallIrvinedLib.ino).
- https://github.com/wonho-maker/Adafruit_SH1106