#include <ArduinoJson.h>
#include <WebServer.h>
#include <ESP32Servo.h>

class RestAPIHAndler{

    public:
        StaticJsonDocument<250> jsonDocument;
        static char buffer[250];
        int pos = 0;
        int previousPos = 0;
        float temperature = 10; // delete hardcoaded value

        WebServer server;

        void create_json(char *tag, float value, char *unit);
        void static getTemperature();
        void handlePostHigh(WebServer server);
        void handlePostLow(WebServer server);
        void handleServo(WebServer server, Servo servo);
};