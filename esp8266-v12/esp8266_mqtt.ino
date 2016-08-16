#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* mqtt_server = "mqtt.thethings.io";

#define TOKEN "YOURTOKEN"

String topic = "v2/things/" + String(TOKEN);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

int cont = 0;

String msg ="{\"values\":[";

void addValue(String key, String value) {
    if (cont == 0) msg += "{\"key\":\""+key+"\",\"value\":\""+value+"\"}";
    else msg +=",{\"key\":\""+key+"\",\"value\":\""+value+"\"}";
    cont++;
}

void addValue(String key, int value) {
    if (cont == 0) msg += "{\"key\":\""+key+"\",\"value\":\""+value+"\"}";
    else msg += ",{\"key\":\""+key+"\",\"value\":\""+value+"\"}";
    cont++;
}

void addValue(String key, float value) {
    if (cont == 0) msg += "{\"key\":\""+key+"\",\"value\":\""+value+"\"}";
    else msg += ",{\"key\":\""+key+"\",\"value\":\""+value+"\"}";
    cont++;
}

void send() {
    msg += "]}";
    client.publish((char*)topic.c_str(),(char*)msg.c_str());
    msg = "{\"values\":[";
    cont = 0;
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe((char*)topic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    addValue("value", (int)random(1,5));
    send();
  }
}
