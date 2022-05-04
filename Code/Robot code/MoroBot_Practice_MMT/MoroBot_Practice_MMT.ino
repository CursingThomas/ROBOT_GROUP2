#define MOROBOT_TYPE morobot_s_rrp // morobot_s_rrr, morobot_s_rrp, morobot_2d, morobot_3d, morobot_p
#define SERIAL_PORT "Serial1"   // "Serial", "Serial1", "Serial2", "Serial3" (not all supported for all microcontroller - see readme)
#define ESP32 ESP32

#include <morobot.h>

MOROBOT_TYPE morobot;   // And change the class-name here

void setup() {
  morobot.begin(SERIAL_PORT);
  morobot.setZero();  // reset angles / moveHome()
  //morobot.moveZAxisIn(); doesnt seem to work well moves too much
        
}

void loop() {

  Serial.println(morobot.getVoltage(0));
  Serial.print("Current [Ampere]: ");
  Serial.println(morobot.getCurrent(0));

  int i = 0;
 
  morobot.setSpeedRPM(1);
  float xMove = 90;
  float yMove = 90;
  
  morobot.moveToAngles(-xMove, yMove, 0); //moves to absolute angles
  morobot.waitUntilIsReady();


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

   /*
   morobot.moveInDirection('z', -20);
   //morobot.moveXYZ(0, 0, -20);
   morobot.waitUntilIsReady();
   morobot.moveInDirection('z', 20);
   //morobot.moveXYZ(0, 0, 20);
   morobot.waitUntilIsReady();
   i = 0;
 */

  while(i < 5){
    i++;
    morobot.moveAngle(0, -30, 10);  
    morobot.waitUntilIsReady();
    morobot.moveAngle(1, -30, 10);
    morobot.waitUntilIsReady();
   }

   morobot.moveHome();

}
