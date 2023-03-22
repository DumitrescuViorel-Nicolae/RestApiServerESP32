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
char buffer[250];
int pos = 0;
int previousPos = 0;
int servoPin = 33;
float temperature = 10; // delete hardcoaded value

// Initialize the webserver on port 80 and servo
WebServer server(80);

//Adafruit_BME680 bme;
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

  
}

void loop(){ 
    server.handleClient(); 

    unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(150); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  Serial.print(F("Temperature = "));
  Serial.print(bme.temperature);
  Serial.println(F(" *C"));

  Serial.print(F("Pressure = "));
  Serial.print(bme.pressure / 100.0);
  Serial.println(F(" hPa"));

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));

  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));

  Serial.print(F("Approx. Altitude = "));
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(F(" m"));

  Serial.println();
  delay(2000);
}