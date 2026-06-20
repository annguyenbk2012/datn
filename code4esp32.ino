#include "config.h"
#include "globals.h"

#include "sensor.h"
#include "door.h"

#include "mcp_tools.h"

#include <esp_task_wdt.h>

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
    float tdsValue,
    bool waterGood
)
{
    Blynk.virtualWrite(V5,temp);
    Blynk.virtualWrite(V6,hum);
    Blynk.virtualWrite(V9,tdsValue);

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

    esp_task_wdt_config_t twdt_config = {
    .timeout_ms = 60000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
    };

    esp_task_wdt_init(&twdt_config);
    esp_task_wdt_add(NULL);

    esp_reset_reason_t reason = esp_reset_reason();
    Serial.print("Reset reason: ");
    Serial.println((int)reason);

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

    // ---------------- Sensor,buzzer, LCD ----------------

    initSensor();

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

    esp_task_wdt_delete(NULL);

    bool result =
    wm.autoConnect(
        "ESP32_SMARTHOME"
    );

    esp_task_wdt_add(NULL);

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

    unsigned long start = millis();

    while(!Blynk.connect())
    {
        delay(500);

        esp_task_wdt_reset();

        if(millis() - start > 30000)
        {
            ESP.restart();
        }
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

    if(WiFi.status() != WL_CONNECTED)
    {
        if(wifiLostTime == 0)
            wifiLostTime = millis();

        WiFi.reconnect();

        if(millis() - wifiLostTime > 60000)
        {
            ESP.restart();
        }
    }
    else
    {
        wifiLostTime = 0;
    }

    if(!mcpClient.isConnected())
    {
        if(mcpLostTime == 0)
            mcpLostTime = millis();

        static unsigned long lastRetry = 0;

        if(millis() - lastRetry > 10000)
        {
            lastRetry = millis();

            mcpClient.begin(
                mcpEndpoint,
                onConnectionStatus
            );
        }

        if(millis() - mcpLostTime > 120000)
        {
            ESP.restart();
        }
    }
    else
    {
        mcpLostTime = 0;
    }

    esp_task_wdt_reset();

}