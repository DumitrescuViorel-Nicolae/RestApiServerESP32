#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Variables and constants declarations
const char* SSID = "Fa-miUnSenvisMic";
const char* PWD = "CaminLaCheie";
StaticJsonDocument<250> jsonDocument;
char buffer[250];
int pos = 0;
int previousPos = 0;
int servoPin = 33;
float temperature = 10; // delete hardcoaded value

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

// Handling the JSON format
void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}

// API functions - GET
void getTemperature() {
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
  server.send(200, "application/json", buffer);
}

// void getEnv() {
//   Serial.println("Get env");
//   jsonDocument.clear();
//   add_json_object("temperature", temperature, "°C");
//   serializeJson(jsonDocument, buffer);
//   server.send(200, "application/json", buffer);
// }

// API functions - POST
void handlePostHigh() {
  // if (server.hasArg("plain") == false) {
  //   //handle error here
  // }
  // String body = server.arg("plain");
  // deserializeJson(jsonDocument, body);
  digitalWrite(26, HIGH);
  server.send(200, "application/json", "{}");
}
void handlePostLow() {
  // if (server.hasArg("plain") == false) {
  //   //handle error here
  // }
  // String body = server.arg("plain");
  // deserializeJson(jsonDocument, body);
  digitalWrite(26, LOW);
  server.send(200, "application/json", "{}");
}

void handleServo(){
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