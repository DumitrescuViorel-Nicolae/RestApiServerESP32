#include <Arduino.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
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
const char *SSID = "ThailandaaaSimple";
const char *PWD = "GarsonieraLaCheie";

StaticJsonDocument<250> jsonDocument;
char buffer[350];
int pos = 0;
int previousPos = 0;
int servoPin = 15;

float temperature;
float pressure;
float humidity;
float iaqReference;
float gas;
float altitude;

// Classes initialisation
AsyncWebServer server(80);
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);
Servo servo;

// Handle WIFI connection
void connectToWifi()
{
  Serial.begin(9600);
  Serial.print("\r\n");
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

// Handling the JSON format
void create_json(char *tag, float value, char *unit)
{
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit)
{
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit;
}

// API functions - GET
void getTemperature()
{
  Serial.println("Get temperature");
  create_json("temperature", temperature, "°C");
}

void getEnvPrincipal()
{
  Serial.println("Get env");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  add_json_object("pressure", pressure, "atm");
  add_json_object("humidity", humidity, "%");
  serializeJson(jsonDocument, buffer);
}

void getEnvSecondary()
{
  jsonDocument.clear();
  add_json_object("gas", gas, "kOhms");
  add_json_object("iaq", iaqReference, "points");
  add_json_object("altitude", altitude, "m");
  serializeJson(jsonDocument, buffer);
}

void handleServo(int position)
{

  if (position > previousPos)
  {
    for (pos = previousPos; pos <= position; pos++)
    {
      servo.write(pos);
      delay(5);
    }
  }
  else
  {
    for (pos = previousPos; pos >= position; pos--)
    {
      servo.write(pos);
      delay(5);
    }
  }
  Serial.println(position);
  previousPos = position;
}

float calculateIAQ(float gasResistance, float humidity)
{
  float humidityScore = humidity / 100.0;
  float gasScore = gasResistance / 100000.0;
  float iaq = 0;

  // Etapa 1: Calculul scorului de umiditate
  // Etapa 1: Calculul scorului de umiditate
  iaq += 0.5 * (humidityScore >= 0.5 ? (100 - humidityScore) : humidityScore);

  // Etapa 2: Calculul scorului de gaz
  iaq += 0.25 * (gasScore >= 0.25 ? (1 - gasScore) : gasScore);

  return iaq;
}

void bmeSensorReadings(void *parameter)
{
  for (;;)
  {
    temperature = bme.readTemperature();
    pressure = bme.readPressure() / 101325;
    humidity = bme.readHumidity();
    gas = bme.readGas() / 1000;
    iaqReference = calculateIAQ(bme.readGas(),humidity);
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    // delay the task
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup_task()
{
  xTaskCreate(
      bmeSensorReadings,
      "Read sensor data",
      3256,
      NULL,
      1,
      NULL);
}

// Server routing and start
void setup_routing()
{

  // GET
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      getTemperature();
      request -> send(200, "application/json", buffer); });
  server.on("/envPrinc", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      getEnvPrincipal();
      request -> send(200, "application/json", buffer); });
  server.on("/envSec", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      getEnvSecondary();
      request -> send(200, "application/json", buffer); });

  // POST
  server.on("/servo", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String json = request->getParam("position")->value();
    int position = json.toInt();
    handleServo(position);
    request -> send(200, "application/json", "Request made"); });

  server.begin();
}

void setup()
{

  servo.attach(servoPin);
  connectToWifi();
  setup_routing();

  // BME sensor setup
  if (!bme.begin())
  {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1)
      ;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_16X);
  bme.setHumidityOversampling(BME680_OS_16X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_7);
  bme.setGasHeater(320, 150);

  setup_task();
}

void loop()
{
}