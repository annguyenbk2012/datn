#ifndef CONFIG_H
#define CONFIG_H


#include <WiFi.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include "WebSocketMCP.h"

// SENSOR
#define DHT_PIN     4
#define DHTTYPE     DHT11

#define TDS_PIN     34
#define GAS_PIN     35
#define FLAME_PIN   15

// RFID
#define SS_PIN      17
#define RST_PIN     5

// DOOR
#define DOOR_LOCK_PIN   25

// OUTPUT
#define FAN_PIN     26
#define LED1_PIN    32
#define LED2_PIN    33

#define ALARM_BUZZER_PIN 13


// ================= WIFI =================
#define RESET_WIFI_PIN 0

// ================= BLYNK VIRTUAL PIN =================
#define V_DOOR            V1
#define V_FAN             V2

#define V_TEMP            V5
#define V_HUM             V6

#define V_TDS             V9
#define V_WATER           V10

#define V_FIRE            V11
#define V_GAS             V12

#define V_LED1            V13
#define V_LED2            V14

#define V_DOOR_PASSWORD   V20

#endif