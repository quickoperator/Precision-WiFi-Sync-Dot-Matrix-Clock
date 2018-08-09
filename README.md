# Precision WiFi Sync Dot Matrix Clock

Part list
- Nodemcu V3
- Max7219 Dot Matrix
- 6v Power adapter
- Hammond 1591D Polycarbonate Box
- Dupont cables

1- Download and install Arduino IDE
    Install ESP8266 library --> Preference --> Additional board manager URL

    Put this URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json

    -->Tools --> Board: --> Boards manager -->Search "esp8266"
    
2- Download and install NodeMCU Py Flasher

3- Download and install ch340g or cp2102 USB driver

4- Flash firmware with firmware "nodemcu-master-22-modules-2018-03-30-21-50-21-float.bin"

5- Open file "ESP_LEDMatrix_clock.ino" and enter your WiFi SSID & Password

6- Upload your project to NodeMCU

7- Connect dupont cables to Max7219

    Vcc Red   + from power adapter
    Gnd Brown  - from power adapter
    D3 Yellow CS
    D5 Green CLK
    D7 Orange DIN
    
8- Connect Gnd (-) & Vin (+) from 6V power adapter

9- Cut box and glue MAx7219 with hot glue !

  
