#include "DistanceSensor.h"
#include <Arduino.h>

DistanceSensor::DistanceSensor(int echopin, int trigpin)
{
    this -> trigPin = trigpin;
    this -> echoPin = echopin;

    pinMode(trigpin, OUTPUT);
    pinMode(echopin, INPUT);
}

void DistanceSensor::watchForObjects()
{
    int prevDistance = distance;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    if (distance < (prevDistance - 10))
    {
        setFlag();
    }
}

void DistanceSensor::setFlag()
{
    flag = true;
}

void DistanceSensor::disableFlag()
{
    flag = false;
}

bool DistanceSensor::getFlag()
{
    return flag;
}