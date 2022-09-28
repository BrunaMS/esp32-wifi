# esp32-wifi
In this project I'm using ESP32 with ESP-IDF framework to implement an Wi-Fi connection, allowing to download 500kb files from a device at least 100m away

## Firmware
### Environment
To develop and build this firmware I'm using VSCode and it's extensions. The main extension required to build it is platformIO but others like git extension can be used to help with version control and gitflow. 

### Task routine
To work with different services/threads I chose to use the  

## Hardware
For now, I'm using KiCad 6.0 to create schematics and PCB for this project. During the development was necessary to download components from external sources. These are:
- Example I
- Example II

### Components 
For this project some circuits are necessary, mainly to guarantee battery charging and to protect other modules and the microcontroller from damages.
- BQ
- USB Protection circuit
- Crystal
- Voltage regulator
- Wi-Fi antenna performance

### Microcontroller
The chosen microcontroller for this application was the ESP32-WROOM, developed to work with IoT and wireless in general, making wireless communication protocols easier to develop and use.

### Routing
The general rules for the routing process were:
1. Consider one manufacturer to define the correct pcb specifications (hole size, clearance, board thickness, impedance control, etc)
2. Always consider grounding rules
3. Avoid interference from power lines
4. Avoid keep high-speed signals close to other signals
