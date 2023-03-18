#include <ArduinoJson.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <RestApiHandler.h>


float* temperature::RestAPIHAndler;

// JSON create
void RestAPIHAndler::create_json(char *tag, float value, char *unit)
{
    jsonDocument.clear();  
    jsonDocument["type"] = tag;
    jsonDocument["value"] = value;
    jsonDocument["unit"] = unit;
    serializeJson(jsonDocument, buffer);
}

// API functions - GET
void RestAPIHAndler::getTemperature()
{
    Serial.println("Get temperature");
    create_json("temperature", temperature, "°C");
    server.send(200, "application/json", buffer);
}

// API functions - POST
void RestAPIHAndler::handlePostHigh(WebServer server)
{
    digitalWrite(26, HIGH);
    server.send(200, "application/json", "{}");
}

void RestAPIHAndler::handlePostLow(WebServer server)
{
     digitalWrite(26, LOW);
    server.send(200, "application/json", "{}");
}

void RestAPIHAndler::handleServo(WebServer server, Servo servo)
{
    String body = server.arg("plain");
    deserializeJson(jsonDocument, body);
    int position = jsonDocument["position"];

    if(position > previousPos){
        for(pos = previousPos; pos<=position; pos++){
        servo.write(pos);
        delay(5);
        }
    }else{
        for(pos = previousPos; pos >= position; pos--){
        servo.write(pos);
        delay(5);
        }
    }

    previousPos = position;
    server.send(200, "application/json", "{}");
}
