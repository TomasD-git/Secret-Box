<img width="830" height="822" alt="image" src="https://github.com/user-attachments/assets/1097ae43-d469-4c11-b35b-7ed5c7d1f3d9" />

> [!NOTE]
> instead of the button there will be a rfid reader

# Full wiring list
## PN532 RFID reader  

VCC -> ESP32 3V3  
GND -> ESP32 GND  
SDA -> ESP32 GPIO8  
SCL -> ESP32 GPIO9  

## Each IRLZ44N MOSFET (one per servo):  

Gate -> ESP32 GPIO6 (Servo 1) / GPIO7 (Servo 2)  
Drain -> Servo GND wire (brown/black)  
Source -> common GND  

## Each SG90 servo:  

Red (VCC) -> Power module +5V  
Brown/Black (GND) -> MOSFET Drain  
Orange (Signal) -> ESP32 GPIO4 (Servo 1) / GPIO5 (Servo 2)  

## Power module:  

+5V output -> ESP32 5V pin + both Servo VCC wires  
GND -> shared GND bus (ESP32 GND, both MOSFET Sources)  
