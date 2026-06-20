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

//========= UpdateBlynkSensors =====================
void updateBlynkSensors(
    float temp,
    float hum,
    float tds,
    bool waterGood
)
{
    Blynk.virtualWrite(V5,temp);
    Blynk.virtualWrite(V6,hum);
    Blynk.virtualWrite(V9,tds);

    Blynk.virtualWrite(
        V10,
        waterGood ? "GOOD" : "BAD"
    );
}

//=============SentAlert ========================
void sendFireAlertEvent()
{
    Blynk.logEvent("fire_alert");
}

void sendGasAlertEvent()
{
    Blynk.logEvent("gas_alert");
}

void sendRFIDDeniedEvent()
{
    Blynk.logEvent("rfid_denied");
}

void updateBlynkFanState(bool state)
{
    Blynk.virtualWrite(
        V_FAN,
        state
    );
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

    // Khi vào AP Mode
    wm.setAPCallback([](WiFiManager *wm)
    {
        lcd.clear();

        lcd.setCursor(0,0);
        lcd.print("AP MODE");

        lcd.setCursor(0,1);
        lcd.print(WiFi.softAPIP());
    });

    bool result =
        wm.autoConnect(
            "ESP32_SMARTHOME"
        );

    if(!result)
    {
        ESP.restart();
    }

    // Da ket noi WiFi thanh cong
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("Connecting to");

    lcd.setCursor(0,1);
    lcd.print(WiFi.SSID());

    delay(3000);
    lcd.clear();


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