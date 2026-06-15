#include "config.h"
#include "globals.h"

#include "sensor.h"
#include "door.h"

#include "mcp_tools.h"

//==============Blynk================
#define BLYNK_TEMPLATE_ID "TMPL6AYTcdamY"
#define BLYNK_TEMPLATE_NAME "An smart home"
#define BLYNK_AUTH_TOKEN "d2-7g4HHepIyEUE_YcCyxGodSGfpT3Vf"

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>

// ======================================================
// BLYNK
// ======================================================

void updateSensors()
{
    if(millis() - lastSensorUpdate < 2000)
        return;

    lastSensorUpdate = millis();

    float temp =
        dht.readTemperature();

    float hum =
        dht.readHumidity();

    if(isnan(temp))
        temp = 0;

    if(isnan(hum))
        hum = 0;

    float tds =
        readTDS(temp);

    int gasValue =
        analogRead(GAS_PIN);

    int flame =
        digitalRead(FLAME_PIN);

    bool fireDetected =
        (flame == 0);

    bool gasDetected =
        (gasValue > 800);

    bool waterGood =
        (tds < 500);

    // ===== BLYNK =====

    Blynk.virtualWrite(V5,temp);
    Blynk.virtualWrite(V6,hum);

    Blynk.virtualWrite(V9,tds);

    Blynk.virtualWrite(
        V11,
        fireDetected ? 1 : 0
    );

    Blynk.virtualWrite(
        V12,
        gasDetected ? 1 : 0
    );

    Blynk.virtualWrite(
        V10,
        waterGood ? "GOOD"
                  : "BAD"
    );

    // ===== FIRE =====

if(fireDetected)
{
    if(!fireAlertSent ||
       millis() - lastFireAlert > 30000)
    {
        Blynk.logEvent("fire_alert");

        fireAlertSent = true;
        lastFireAlert = millis();
    }
}
else
{
    fireAlertSent = false;
}

    // ===== GAS =====

if(gasDetected)
{
    if(!gasAlertSent ||
       millis() - lastGasAlert > 30000)
    {
        Blynk.logEvent("gas_alert");

        gasAlertSent = true;
        lastGasAlert = millis();
    }
}
else
{
    gasAlertSent = false;
}


    // ===== WATER =====

if(!waterGood)
{
    if(!waterAlertSent ||
       millis() - lastWaterAlert > 60000)
    {
        Blynk.logEvent("water_bad");

        waterAlertSent = true;
        lastWaterAlert = millis();
    }
}
else
{
    waterAlertSent = false;
}

    // ===== ALARM =====

    if(fireDetected ||
       gasDetected)
    {
        alarmOn();
    }
    else
    {
        alarmOff();
    }

    // ===== LCD =====

    lcd.setCursor(0,0);

    lcd.print("T:");
    lcd.print(temp);

    lcd.print(" H:");
    lcd.print(hum);

    lcd.print("   ");

    lcd.setCursor(0,1);

    if(waterGood)
        lcd.print("Water: GOOD ");
    else
        lcd.print("Water: BAD  ");
}

// ======================================================
// SYNC BLYNK
// ======================================================

void syncBlynk()
{
    Blynk.virtualWrite(
        V_LED1,
        led1State
    );

    Blynk.virtualWrite(
        V_LED2,
        led2State
    );

    Blynk.virtualWrite(
        V_FAN,
        fanState
    );
}

BLYNK_WRITE(V_LED1)
{
    led1State = param.asInt();
    digitalWrite(LED1_PIN, led1State);
}

BLYNK_WRITE(V_LED2)
{
    led2State = param.asInt();
    digitalWrite(LED2_PIN, led2State);
}

BLYNK_WRITE(V_FAN)
{
    fanState = param.asInt();
    digitalWrite(FAN_PIN, fanState );
}

BLYNK_WRITE(V_DOOR_PASSWORD)
{
    enteredPassword =
        param.asStr();
}

BLYNK_WRITE(V_DOOR)
{
    if(param.asInt())
    {
        if(enteredPassword ==
           doorPassword)
        {
            openDoor();

            enteredPassword = "";

            Blynk.virtualWrite(
                V_DOOR_PASSWORD,
                ""
            );
        }
        else
        {
            Blynk.logEvent(
                "door_wrong_password"
            );
        }
    }
}

BLYNK_CONNECTED()
{
    Blynk.syncAll();
}

// ================= STATE =================
bool led1State = false;
bool led2State = false;
bool fanState  = false;

bool gasAlertSent   = false;
bool fireAlertSent  = false;
bool waterAlertSent = false;


// ======================================================
// MCP CALLBACK
// ======================================================

void onConnectionStatus(
    bool connected)
{
    if(connected)
    {
        Serial.println(
            "[MCP] Connected"
        );

        registerMcpTools();
    }
    else
    {
        Serial.println(
            "[MCP] Disconnected"
        );
    }
}


// ======================================================
// SETUP
// ======================================================

void setup()
{
    Serial.begin(115200);

    // ---------------- LED ----------------

    pinMode(
        LED1_PIN,
        OUTPUT
    );

    pinMode(
        LED2_PIN,
        OUTPUT
    );

    digitalWrite(
        LED1_PIN,
        LOW
    );

    digitalWrite(
        LED2_PIN,
        LOW
    );

    // ---------------- FAN ----------------

    pinMode(
        FAN_PIN,
        OUTPUT
    );

    digitalWrite(
        FAN_PIN,
        LOW
    );

    // ---------------- BUZZER ----------------

    pinMode(
        ALARM_BUZZER_PIN,
        OUTPUT
    );

    digitalWrite(
        ALARM_BUZZER_PIN,
        LOW
    );

    // ---------------- FLAME ----------------

    pinMode(
        FLAME_PIN,
        INPUT
    );

    // ---------------- DHT ----------------

    dht.begin();

    // ---------------- LCD ----------------

    lcd.init();
    lcd.backlight();

    lcd.setCursor(0,0);
    lcd.print("SMART HOME");

    delay(1500);

    lcd.clear();

    // ---------------- DOOR ----------------

    initDoor();

    // ---------------- RFID ----------------

    initRFID();

    // ---------------- WIFI RESET ----------------

    pinMode(
        RESET_WIFI_PIN,
        INPUT_PULLUP
    );

    if(
        digitalRead(
            RESET_WIFI_PIN
        ) == LOW
    )
    {
        WiFiManager wm;

        wm.resetSettings();

        ESP.restart();
    }

    // ---------------- WIFI ----------------

    WiFiManager wm;

    bool result =
        wm.autoConnect(
            "ESP32_SMARTHOME"
        );

    if(!result)
    {
        ESP.restart();
    }

    Serial.println(
        WiFi.localIP()
    );

    // ---------------- BLYNK ----------------

    Blynk.config(
        BLYNK_AUTH_TOKEN
    );

    while(!Blynk.connect())
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println(
        "\nBlynk Connected"
    );

    // ---------------- MCP ----------------

    mcpClient.begin(
        mcpEndpoint,
        onConnectionStatus
    );
}

// ======================================================
// LOOP
// ======================================================

void loop()
{
    Blynk.run();

    mcpClient.loop();

    handleRFID();

    handleDoor();
    updateSensors();

}