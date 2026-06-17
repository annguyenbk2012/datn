#include <Arduino.h>
#include <SPI.h>
#include "door.h"
#include "globals.h"
#include "config.h"

bool unlocking = false;
unsigned long unlockTime = 0;

void initDoor()
{
    pinMode(DOOR_LOCK_PIN, OUTPUT);

    digitalWrite(
        DOOR_LOCK_PIN,
        LOW
    );
}

void openDoor()
{
    Serial.println("[DOOR] OPEN");

    digitalWrite(DOOR_LOCK_PIN, HIGH);

    unlocking = true;
    unlockTime = millis();
}
void handleDoor()
{
    if(unlocking &&
       millis() - unlockTime > 1000)
    {
        Serial.println("[DOOR] CLOSE");

        digitalWrite(
            DOOR_LOCK_PIN,
            LOW
        );

        unlocking = false;
    }
}


String getUID();
void loadCards();
void saveCard(String uid);
bool cardExists(String uid);

void initRFID()
{
    SPI.begin();

    rfid.PCD_Init();

    prefs.begin("rfid_db", false);

    loadCards();

    Serial.println("[RFID] Ready");
}

void loadCards()
{
    cardCount = prefs.getInt("count",0);

    for(int i=0;i<cardCount;i++)
    {
        String key="card"+String(i);

        authorizedCards[i] =
        prefs.getString(key.c_str(),"");
    }
}

bool cardExists(String uid)
{
    for(int i=0;i<cardCount;i++)
    {
        if(authorizedCards[i]==uid)
            return true;
    }

    return false;
}

void saveCard(String uid)
{
    if(cardCount>=50) return;

    String key="card"+String(cardCount);

    prefs.putString(key.c_str(),uid);

    authorizedCards[cardCount]=uid;

    cardCount++;

    prefs.putInt("count",cardCount);
}

String getUID()
{
    String uid="";

    for(byte i=0;i<rfid.uid.size;i++)
    {
        if(rfid.uid.uidByte[i] < 0x10)
            uid+="0";

        uid+=String(
            rfid.uid.uidByte[i],
            HEX
        );
    }

    uid.toUpperCase();

    return uid;
}

void handleRFID()
{
    if(!rfid.PICC_IsNewCardPresent())
        return;

    if(!rfid.PICC_ReadCardSerial())
        return;

    String uid=getUID();

    if(uid==managerUID)
    {
        enrollMode=true;

        Serial.println("[RFID] Scan new card");

        rfid.PICC_HaltA();

        return;
    }

    if(enrollMode)
    {
        if(!cardExists(uid))
        {
            saveCard(uid);

            Serial.println("[RFID] Card Added");
        }

        enrollMode=false;

        rfid.PICC_HaltA();

        return;
    }

    if(cardExists(uid))
    {
        openDoor();
    }
    else
    {
        Serial.println("[RFID] Access Denied");
    }

    rfid.PICC_HaltA();
}