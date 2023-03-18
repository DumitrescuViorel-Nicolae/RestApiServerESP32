#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <RestAPIHandler.h>

// Variables and constants declarations
const char* SSID = "Fa-miUnSenvisMic";
const char* PWD = "CaminLaCheie";

// Initialize the webserver on port 80 and servo
WebServer server(80);
Servo servo;

// Handle WIFI connection
void connectToWifi(){
    Serial.begin(9600);
    Serial.print("\r\n");
    Serial.print("Connecting to ");
    Serial.println(SSID);

    WiFi.begin(SSID, PWD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
  }
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
}

// Server routing and start
void setup_routing() {
  // GET methods
  server.on("/temperature", getTemperature);	 	 	 
  //server.on("/env", getEnv);	 	 

  // POST methods
  server.on("/ledHigh", handlePostHigh);
  server.on("/ledLow", handlePostLow);
  server.on("/servo", HTTP_POST, handleServo);

  server.begin();	 	 
}

void setup(){
    pinMode(26, OUTPUT);
    digitalWrite(26, LOW);
    servo.attach(servoPin);
    connectToWifi();
    setup_routing();
}

void loop(){
    server.handleClient();
     
}