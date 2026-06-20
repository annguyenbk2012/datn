#ifndef SENSOR_H
#define SENSOR_H

float readTDS(float temp);

void initSensor();
void alarmOn();
void alarmOff();
void updateSensors();

extern float currentTemp;
extern float currentHum;

extern bool gasDetected;
extern bool fireDetected;
extern bool waterGood;
#endif