#define MOROBOT_TYPE   morobot_s_rrp // morobot_s_rrr, morobot_s_rrp, morobot_2d, morobot_3d, morobot_p
#define SERIAL_PORT   "Serial1"   // "Serial", "Serial1", "Serial2", "Serial3" (not all supported for all microcontroller - see readme)
#define ESP32 ESP32

#include <morobot.h>

#include <WiFi.h>
#include <PubSubClient.h>

MOROBOT_TYPE morobot;   // And change the class-name here
String messageTemp;
char* Topic;

const char* ssid = "Sara";
const char* password = "12345678";

const char* mqtt_server = "mqtt.eclipseprojects.io";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  morobot.begin(SERIAL_PORT);
  morobot.setZero();  // reset angles / moveHome()
  //morobot.moveZAxisIn(); doesnt seem to work well moves too much     // Set the global speed for all motors here. This value can be overwritten temporarily if a function is called with a speed parameter explicitely.

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

  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
  {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("ESP8266Client")) 
      {
        Serial.println("connected");
        // Subscribe
        client.subscribe("esp32/output");
      } 
      else 
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
  }
}

//gets the message and sends it to the main loop
void callback(char* topic, byte* message, unsigned int length) 
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Topic = topic;
  Serial.print(". Message: ");
  
  for (int i = 0; i < length; i++) 
  {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
  }
  
  Serial.println();
}


void loop() {

  if (!client.connected()) 
  {
    reconnect();
  }

  int i = 0;
 
  morobot.setSpeedRPM(1);
  float xMove = 90;
  float yMove = 90;


  if (String(Topic) == "Fruitsystem/color")
  { 
    Serial.print("I got something");
    if(messageTemp == "1")//the number it gets when red
    { 
      Serial.println("Ripe");
      morobot.moveToAngles(-xMove, yMove, 0); //moves to absolute angles
      morobot.waitUntilIsReady();
    }
    else if(messageTemp == "0")//the number it gets when green
    {
      Serial.println("Unripe");
      morobot.moveToAngles(xMove, -yMove, 0);
      morobot.waitUntilIsReady();
    }
  }
   
   client.loop();
}
