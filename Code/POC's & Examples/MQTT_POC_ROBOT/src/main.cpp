#include <Arduino.h>
#include "PubSubClient.h"  
#include "ArduinoJson.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "Internet";
const char* password = "TLE9009tle";

const char* mqtt_server = "192.168.2.254";
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup() 
{
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
}

void loop() 
{
}