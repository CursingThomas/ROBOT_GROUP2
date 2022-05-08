#include <Arduino.h>
#include <morobot.h>

#include <WiFi.h>
#include <PubSubClient.h>

#include <../include/buffer.h>

#define MOROBOT_TYPE morobot_s_rrp // morobot_s_rrr, morobot_s_rrp, morobot_2d, morobot_3d, morobot_p
#define SERIAL_PORT "Serial1"	   // "Serial", "Serial1", "Serial2", "Serial3" (not all supported for all microcontroller - see readme)
//#define ESP32         ESP32

MOROBOT_TYPE morobot; // And change the class-name here

String Topic;
Buffer buffer = Buffer();

const char *ssid = "Sara";		   // change to your own ssid
const char *password = "12345678"; // change to your own password

const char *mqtt_server = "mqtt.eclipseprojects.io";

WiFiClient espClient;
PubSubClient client(espClient);

bool datapushed = false;

void setup_wifi();
void reconnect();
void callback(char *topic, byte *payload, unsigned int length);

void setup()
{
	morobot.begin(SERIAL_PORT);
	morobot.setZero(); // reset angles / moveHome()
	// morobot.moveZAxisIn(); doesnt seem to work well moves too much     // Set the global speed for all motors here. This value can be overwritten temporarily if a function is called with a speed parameter explicitely.

	Serial.begin(115200);

	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
}

void loop()
{

	if (!client.connected())
	{
		reconnect();
	}

	// morobot.setSpeedRPM(50);
	// float xMove = 45;
	// float yMove = 45;

	// If the buffer is not empty, pop the first message and move the robot
	if (datapushed == true && buffer.IsEmpty() == false)
	{
		String message = buffer.pop();
		Serial.println("Message = " + message);
		if (message == "1") // the number it gets when red
		{
			Serial.println("Ripe");
			// morobot.moveToAngles(-xMove, yMove, 0); //moves to absolute angles
			// morobot.waitUntilIsReady();
			Serial.println(buffer.get(1));
			client.publish("Fruitsystem/robot", "OnPosition");
			message = "";
		}
		else if (message == "0") // the number it gets when green
		{
			Serial.println("Unripe");
			// morobot.moveToAngles(xMove, -yMove, 0);
			// morobot.waitUntilIsReady();
			Serial.println(buffer.get(1));
			client.publish("Fruitsystem/robot", "OnPosition");
			message = "";
		}
		Topic = "";
		
	}

	client.loop();
}

void setup_wifi()
{
	delay(10);

	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	// wait for connection
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

void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected())
	{
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("Robot"))
		{
			Serial.println("connected");
			client.subscribe("Fruitsystem/color"); // subscribe to the topic DO NOT CHANGE
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

// gets the message and puts it in the buffer
void callback(char *topic, byte *message, unsigned int length)
{
	String messageTemp;
	Serial.print("Message arrived on topic: ");
	Serial.print(topic);
	Topic = String(topic);
	Serial.print(". Message: ");

	for (int i = 0; i < length; i++)
	{
		Serial.print((char)message[i]);
		messageTemp += (char)message[i];
	}

	// add the message to the buffer
	datapushed = buffer.push(messageTemp);
}