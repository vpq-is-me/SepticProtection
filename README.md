# Overfill protection for Sewage Treatment System

This device used for protection autonomous Sewage Treatment System (STS) ТОПАС-5 (Topas 5) as on picture:

### Topas 5 example
![Alt text](hardware/topas5_2kompressora.jpg?raw=true)

For level overfill measure used 2 float switches: for Incame tank and Drain tank. 
Circuit diagram shown bellow.

### Circuit diagram  
![Alt text](hardware/diagramm.png?raw=true)

### PCB view  
![Alt text](hardware/pcb_view.jpg?raw=true)

Controller ESP32 connected to Bluetooth MESH and broadcast state to [home_automation_server](https://github.com/vpq-is-me/home_auto_nodejs.git) (running on Raspberry Pi) and __most importantly__ to [water pump controller](https://github.com/vpq-is-me/WaterPumpController_ESP32.git) to stop water feeding in case of STS failure. 