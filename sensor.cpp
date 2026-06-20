#include <Arduino.h>
#include "sensor.h"
#include "globals.h"
#include "config.h"

void alarmOn()
{
    digitalWrite(ALARM_BUZZER_PIN, HIGH);
}

void alarmOff()
{
    digitalWrite(ALARM_BUZZER_PIN, LOW);
}

float readTDS(float temperature)
{
    int analogValue = analogRead(TDS_PIN);

    float voltage =
        analogValue * 3.3 / 4095.0;

    float compensationCoefficient =
        1.0 + 0.02 * (temperature - 25.0);

    float compensationVoltage =
        voltage / compensationCoefficient;

    float tdsRaw =
        (133.42 * pow(compensationVoltage, 3)
        -255.86 * pow(compensationVoltage, 2)
        +857.39 * compensationVoltage)
        * 0.5;

    return tdsRaw;
}

void initSensor()
{
    pinMode(FLAME_PIN, INPUT);

    pinMode(ALARM_BUZZER_PIN, OUTPUT);
    digitalWrite(ALARM_BUZZER_PIN, LOW);

    dht.begin();

    lcd.init();
    lcd.backlight();

    lcd.setCursor(0,0);
    lcd.print("SMART HOME");

    delay(1500);

    lcd.clear();
}

float currentTemp = 0;
float currentHum = 0;
bool fireDetected = false;
bool gasDetected = false;
bool waterGood = true;

//=================Update Sensor=====================
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

    if(!isnan(temp))
    {
        Serial.printf("Temp: %.2f\n",temp);
        temp =
            tempKalman.updateEstimate(
                temp
            );
        currentTemp = temp;
        Serial.printf("TempKalman: %.2f\n",currentTemp);
    }

    if(!isnan(hum))
    {
        Serial.printf("Hum: %.2f\n",hum);
        hum =
            humKalman.updateEstimate(
                hum
            );
        currentHum = hum;
        Serial.printf("HumKalman: %.2f\n",currentHum);
    }

    float tdsRaw =
        readTDS(temp);

    float tdsValue =
        tdsKalman.updateEstimate(
            tdsRaw
        );
    
    Serial.printf(
    "tdsRaw=%d tdsKalman=%.0f\n",
    tdsRaw,
    tdsValue
    );

    int gasRaw =
        analogRead(GAS_PIN);

    float gasValue =
        gasKalman.updateEstimate(
            gasRaw
        );

    Serial.printf(
    "gasRaw=%d gasKalman=%.0f\n",
    gasRaw,
    gasValue
    );

    int flame = digitalRead(FLAME_PIN);

    fireDetected =
    (flame == 0);

    gasDetected =
    (gasValue > 800);
    
    waterGood =
    (tdsValue < 500);

    // ===== BLYNK =====

    updateBlynkSensors(
        temp,
        hum,
        tdsValue,
        waterGood
    );

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


    // ===== FIRE =====

    if(fireDetected)
    {
        if(!fireAlertSent || millis() - lastFireAlert > 30000)
        {
            sendFireAlertEvent();

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
        if(!gasAlertSent || millis() - lastGasAlert > 30000)
        {
            sendGasAlertEvent();

            gasAlertSent = true;
            lastGasAlert = millis();
        }
    }
    else
    {
        gasAlertSent = false;
    }

    // ===== ALARM =====

    if(gasDetected)
    {
        fanState = true;
        digitalWrite(FAN_PIN, HIGH);
        updateBlynkFanState(true);
    
    }

    if(fireDetected || gasDetected)
    {
        alarmOn();
    }
    else
    {
        alarmOff();
    }
}
