#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"


#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5

// Variables and constants declarations
const char* SSID = "Fa-miUnSenvisMic";
const char* PWD = "CaminLaCheie";

StaticJsonDocument<250> jsonDocument;
char buffer[350];
int pos = 0;
int previousPos = 0;
int servoPin = 33;

float temperature;
float pressure;
float humidity;
float gas;
float altitude;

// Classes initialisation
WebServer server(80);
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);
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

void getEnvPrincipal() {
  Serial.println("Get env");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  add_json_object("pressure", pressure, "mBar");
  add_json_object("humidity", humidity, "%");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void getEnvSecondary(){
  jsonDocument.clear();

  add_json_object("gas", gas, "kOhms");
  add_json_object("altitude", altitude, "m");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

// API functions - POST
void handlePostHigh() {
  digitalWrite(26, HIGH);
  server.send(200, "application/json", "{}");
}

void handlePostLow() {
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

  // GET
  server.on("/temperature", getTemperature);	 	 	 
  server.on("/envPrinc", getEnvPrincipal);	 	 
  server.on("/envSec", getEnvSecondary);

  // POST
  server.on("/ledHigh", handlePostHigh);
  server.on("/ledLow", handlePostLow);
  server.on("/servo", HTTP_POST, handleServo);
  server.begin();	 	 
}

void bmeSensorReadings(void * parameter){
   for (;;) {
      temperature = bme.readTemperature();
      pressure = bme.readPressure();
      humidity = bme.readHumidity();
      gas = bme.readGas();
      altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
      Serial.println("Read sensor data");
      Serial.print(gas);
     // delay the task
     vTaskDelay(10000 / portTICK_PERIOD_MS);
   }
}

void setup_task() {	 	 
  xTaskCreate(	 	 
  bmeSensorReadings, 	 	 
  "Read sensor data", 	 	 
  1000, 	 	 
  NULL, 	 	 
  1, 	 	 
  NULL 	 	 
  );	 	 
}

void setup(){
    pinMode(26, OUTPUT);
    digitalWrite(26, LOW);
    servo.attach(servoPin);
    connectToWifi();
    setup_routing();

  // BME sensor setup
   if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    setup_task();
}

void loop(){ 
    server.handleClient();
}