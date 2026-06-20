#ifndef GLOBALS_H
#define GLOBALS_H

#pragma once
#include "config.h"
#include "WebSocketMCP.h"
#include <SimpleKalmanFilter.h>

extern DHT dht;
extern LiquidCrystal_I2C lcd;

extern MFRC522 rfid;
extern Preferences prefs;

extern WebSocketMCP mcpClient;

extern const char* mcpEndpoint;

extern bool led1State;
extern bool led2State;
extern bool fanState;

extern SimpleKalmanFilter tempKalman;
extern SimpleKalmanFilter humKalman;
extern SimpleKalmanFilter gasKalman;
extern SimpleKalmanFilter tdsKalman;

extern bool gasAlertSent;
extern bool fireAlertSent;

extern unsigned long wifiLostTime;
extern unsigned long mcpLostTime;

extern String managerUID;
extern String authorizedCards[50];
extern int cardCount;
extern bool enrollMode;
extern String doorPassword;
extern String enteredPassword;

extern unsigned long lastGasAlert;
extern unsigned long lastFireAlert;
extern unsigned long lastWaterAlert;

extern unsigned long lastSensorUpdate;
#pragma once

void syncBlynk();

void updateBlynkSensors(
    float temp,
    float hum,
    float tdsValue,
    bool waterGood
);

void sendFireAlertEvent();
void sendGasAlertEvent();
void sendWaterBadEvent();

void updateBlynkFanState(bool state);

void sendRFIDDeniedEvent();
void sendWrongPasswordEvent();

#endif