#include "EspMQTTClient.h"
#include <PubSubClient.h>
#include <Arduino.h>

EspMQTTClient client(
  "12connect",
  "",
  "mqtt.eclipseprojects.io", // MQTT Broker
  "ThomasESP",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void onConnectionEstablished()
{
  client.subscribe("BIPDemo/Messages", [](const String & payload) {
    Serial.println(payload);
  });
  client.publish("BIPDemo/Messages", "I am connected!");
}


void setup()
{
  Serial.begin(115200);
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("BIPDemo/Messages", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
  
  
}

void loop()
{
  client.loop();
  //client.publish("BIPDemo/Ack", "Message sent from esp");
  //delay(5000);
}