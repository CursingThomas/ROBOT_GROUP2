#include "EspMQTTClient.h"
#include <Arduino.h>

EspMQTTClient client(
  "Internet",
  "TLE9009tle",
  "test.mosquitto.org", // MQTT Broker
  "ThomasESP",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void onConnectionEstablished()
{
  client.subscribe("Test/mqtt", [](const String & payload) {
    Serial.println(payload);
  });
  client.publish("Test/mqtt", "This is a message from client1");
}


void setup()
{
  Serial.begin(115200);
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("Test/mqtt", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
  
}

void loop()
{
  client.loop();
  client.publish("Test/mqtt", "Message sent from esp");
  delay(1000);
}