#define MOROBOT_TYPE   morobot_s_rrp // morobot_s_rrr, morobot_s_rrp, morobot_2d, morobot_3d, morobot_p
#define SERIAL_PORT   "Seria2"   // "Serial", "Serial1", "Serial2", "Serial3" (not all supported for all microcontroller - see readme)
//#define ESP32 ESP32

#include <morobot.h>

MOROBOT_TYPE morobot;   // And change the class-name here

const int BUTTON1 = 2;
const int BUTTON2 = 4;

int BUTTONstate1 = 0;
int BUTTONstate2 = 0;

void setup() {
  morobot.begin(SERIAL_PORT);
  morobot.setZero();  // reset angles / moveHome()
  //morobot.moveZAxisIn(); doesnt seem to work well moves too much     // Set the global speed for all motors here. This value can be overwritten temporarily if a function is called with a speed parameter explicitely.

  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
}

void loop() {

  Serial.println(morobot.getVoltage(0));
  Serial.print("Current [Ampere]: ");
  Serial.println(morobot.getCurrent(0));

  int i = 0;
 
  morobot.setSpeedRPM(1);
  float xMove = 90;
  float yMove = 90;

  BUTTONstate1 = digitalRead(BUTTON1);
  if (BUTTONstate1 == HIGH)
  {
  morobot.moveToAngles(-xMove, yMove, 0); //moves to absolute angles
  morobot.waitUntilIsReady();
  }

  morobot.moveToAngles(xMove, -yMove, 0);
  morobot.waitUntilIsReady();
  
  morobot.moveHome();

  
  while(i < 3){
    i++;
    morobot.moveAngle(0, 30, 10);  //move motor one
    morobot.waitUntilIsReady();
    morobot.moveAngle(1, 30, 10); //move motor two
    morobot.waitUntilIsReady();
   }


  while(i < 5){
    i++;
    morobot.moveAngle(0, -30, 10);  
    morobot.waitUntilIsReady();
    morobot.moveAngle(1, -30, 10);
    morobot.waitUntilIsReady();
   }

   morobot.moveHome();

  

}
