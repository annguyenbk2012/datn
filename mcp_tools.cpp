#include <Arduino.h>
#include <ArduinoJson.h>

#include "mcp_tools.h"
#include "globals.h"
#include "config.h"
#include "sensor.h"
#include "door.h"

void registerMcpTools()
{
    // ================= LED1 =================

    mcpClient.registerTool(
        "room_living_control",
        "Bat tat den phong khach",

        R"({
            "type":"object",
            "properties":{
                "state":{
                    "type":"string",
                    "enum":["on","off"]
                }
            },
            "required":["state"]
        })",

        [](const String& args)
        {
            DynamicJsonDocument doc(128);

            deserializeJson(doc,args);

            String state =
                doc["state"];

            led1State =
                (state == "on");

            digitalWrite(
                LED1_PIN,
                led1State
            );

            syncBlynk();

            return WebSocketMCP::
            ToolResponse(
                "{\"success\":true}"
            );
        });

    Serial.println(
        "[MCP] LED1 ready"
    );

    // ================= LED2 =================

    mcpClient.registerTool(
        "room_kitchen_control",
        "Bat tat den phong bep",

        R"({
            "type":"object",
            "properties":{
                "state":{
                    "type":"string",
                    "enum":["on","off"]
                }
            },
            "required":["state"]
        })",

        [](const String& args)
        {
            DynamicJsonDocument doc(128);

            deserializeJson(doc,args);

            String state =
                doc["state"];

            led2State =
                (state == "on");

            digitalWrite(
                LED2_PIN,
                led2State
            );

            syncBlynk();

            return WebSocketMCP::
            ToolResponse(
                "{\"success\":true}"
            );
        });

    Serial.println(
        "[MCP] LED2 ready"
    );

    // ================= FAN =================

    mcpClient.registerTool(
        "fan_control",
        "Dieu khien quat",

        R"({
            "type":"object",
            "properties":{
                "state":{
                    "type":"string",
                    "enum":["on","off"]
                }
            },
            "required":["state"]
        })",

        [](const String& args)
        {
            DynamicJsonDocument doc(128);

            deserializeJson(doc,args);

            String state =
                doc["state"];

            fanState =
                (state == "on");

            digitalWrite(
                FAN_PIN,
                fanState
            );

            syncBlynk();

            return WebSocketMCP::
            ToolResponse(
                "{\"success\":true}"
            );
        });

    Serial.println(
        "[MCP] FAN ready"
    );


    // ================= SENSOR =================

    mcpClient.registerTool(
        "get_environment_data",
        "Lay du lieu moi truong",

        R"({
            "type":"object",
            "properties":{}
        })",

        [](const String& args)
        {
            DynamicJsonDocument doc(256);

            doc["temperature"] =
                currentTemp;

            doc["humidity"] =
                currentHum;

            doc["water_quality"] =
                waterGood ?
                "GOOD" :
                "BAD";

            doc["gas"] =
                gasDetected ?
                "DETECTED" :
                "SAFE";

            doc["fire"] =
                fireDetected ?
                "DETECTED" :
                "SAFE";

            String response;

            serializeJson(
                doc,
                response
            );

            return WebSocketMCP::
            ToolResponse(
                response
            );
        });

    Serial.println(
        "[MCP] SENSOR ready"
    );
}