# ESP8266 - v12

Here's an example of sending messages to thethings.iO using an esp8266 v12 (WEMOS D1 mini).

Clone this repository and copy all the code from the .ino file to the arduino IDE.

Remember to change "YOURTOKEN" with your actual thing token, SSID with your wifi ssid and password with your wifi password.
```
const char* ssid = "yourSSID";
const char* password = "wifipassword";

#define TOKEN "YOURTOKEN"
```

Add values and send them using the addValue (1 or +) and send function. 
```
addValue("value", 5); 
send();
```
Actually, the mqtt buffer is limited by 128 bytes. If you want to send multiple values, change the ```#define MQTT_MAX_PACKET_SIZE 128``` (PubSubClient.h file, inside the arduino PubSubClient library) to a bigger number (256, 512...).

##esp8266 arduino library 

- Install Arduino 1.6.8 (or greater) from the Arduino website.
- Start Arduino and open Preferences window.
- Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
- Open Boards Manager from Tools > Board menu and install esp8266 platform (and don't forget to select your ESP8266 board from Tools > Board menu after installation).

Before flashing the firmware to the esp, remember to put the board in flash mode. From Tools, select the board acording to your vendor (Wemos D1 mini in our case), select 115200 as the speed and select the proper port.
