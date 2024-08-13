# C/C++ Code for controlling a bidirectional charger for electric vehicle providing vehicle-to-grid services
> By Jonathan Dumaz

## General aim of the code
This code is written in C/C++ for an Arduino Uno controller. Its aim is to control the powering part of the charger, allowing charge or discharge if possible. It uses data from current and voltage sensors. It is also configured to read CAN frames coming from the battery manager system. Finally there is also code for a little HMI with two buttons and a screen.

## Algorithm followed by the code
The code is following the basic algorithm visible on the diagram below. The aim is to charge or discharge the battery with autorization of the battery  manager system.
![diagram of the charger's algorithm](<procedure for charging_discharging.png>)

## Libraries used
This code is using two publics libraries.
- The first one is for controlling the LCD screen by I2C. Its github is here: https://github.com/johnrickman/LiquidCrystal_I2C.
- It is also using a library for controlling by SPI the MCP2515_CAN module. Its github is here: https://github.com/autowp/arduino-mcp2515?tab=readme-ov-file#initialization.
- The last library has been created for this project, to collect data from the current sensor.

