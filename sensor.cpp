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

    float tds =
        (133.42 * pow(compensationVoltage, 3)
        -255.86 * pow(compensationVoltage, 2)
        +857.39 * compensationVoltage)
        * 0.5;

    return tds;
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
