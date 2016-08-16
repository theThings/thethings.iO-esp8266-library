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
