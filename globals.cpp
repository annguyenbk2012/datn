#include "globals.h"

SimpleKalmanFilter tempKalman(1,1,0.1);
SimpleKalmanFilter humKalman(1,1,0.1);
SimpleKalmanFilter gasKalman(10,10,0.5);

bool doorOpened = false;
unsigned long doorTime = 0;

String managerUID = "C941F605";

String authorizedCards[50];
int cardCount = 0;

bool enrollMode = false;

unsigned long lastSensorUpdate = 0;
unsigned long lastGasAlert = 0;
unsigned long lastFireAlert = 0;
unsigned long lastWaterAlert = 0;

bool led1State = false;
bool led2State = false;
bool fanState  = false;

bool gasAlertSent   = false;
bool fireAlertSent  = false;

unsigned long wifiLostTime = 0;
unsigned long mcpLostTime = 0;


String doorPassword = "12345";
String enteredPassword = "";

LiquidCrystal_I2C lcd(0x27,16,2);
DHT dht(DHT_PIN,DHTTYPE);

MFRC522 rfid(SS_PIN,RST_PIN);

Preferences prefs;

WebSocketMCP mcpClient;

const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?token=eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOjkwMDAwNCwiYWdlbnRJZCI6MTczNDIyMSwiZW5kcG9pbnRJZCI6ImFnZW50XzE3MzQyMjEiLCJwdXJwb3NlIjoibWNwLWVuZHBvaW50IiwiaWF0IjoxNzc4NTg0MDgzLCJleHAiOjE4MTAxNDE2ODN9.izOjJPOT0nzRuYdNaAEeQ-TYB7p5HWc_TRyUpcdQBmZ5rrY5a0Hus38VY7khr6NOxdK4htzlYirs7hXWIvDoxQ";