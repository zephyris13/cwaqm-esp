#include <ESP8266WiFi.h> 
#include <WiFiClient.h> 
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h> 
#include "Adafruit_CCS811.h"
#include "DHT.h"

#define DHT11_PIN 14
#define DHTTYPE DHT11

WiFiClient client;
Adafruit_CCS811 ccs;
DHT dht(DHT11_PIN, DHTTYPE);

String blankPage = "<html></html>";

int co2 = 0;
int tvoc = 0;
float temp = 0;

String host = "cwaqm-backend.azurewebsites.net";
String protocol = "http://";
int port = 80;
String userId = "<your-user-key-here>";

void queryController()
{
    Serial.println("DHT11 Sensor Data");
      
    temp = dht.readTemperature();

    bool dhtError = isnan(temp);

    if (dhtError)
    {
        Serial.println("DHT11 Sensor Error!");
    }
  
    if (ccs.available() && !dhtError)
    {
        Serial.println("CCS811 Sensor Data");
      
        ccs.setTempOffset(temp);
        
        if (!ccs.readData())
        {
            co2 = ccs.geteCO2();
            tvoc = ccs.getTVOC();

            Serial.println("Temp: " + String(temp) + " |CO2: " + String(co2) + " |TVOC: " + String(tvoc));
        }
        else
        {
            Serial.println("CCS811 Sensor Error!");
        }
    }
    else
    {
        Serial.println("CCS811 Sensor Not Found!");
    }
}

void httpPost()
{
    HTTPClient http;
 
    http.begin(protocol + host + ":" + String(port) + "/api/logs");
    http.addHeader("Content-Type", "application/json; charset=utf-8");
 
    int httpCode = http.POST(jsonOutput());
    
    Serial.println(httpCode);
 
    http.end();
}

String jsonOutput()
{
    String jsonString = "{";

    jsonString += "\"userid\":\"" + userId + "\",";
    jsonString += "\"co2\":" + String(co2) + ",";
    jsonString += "\"tvoc\":" + String(tvoc) + ",";
    jsonString += "\"temp\":" + String(temp);

    jsonString += "}";

    return jsonString;
}

void mainLoop()
{
    delay(30 * 1000);
    while (!client.connect(host, port))
    {
        Serial.println("Connection to host failed");
        delay(10000);
    }

    queryController();
    Serial.println("Sending data...");
    // POST data
    httpPost();

    Serial.println("Entering sleep mode");
    ESP.deepSleep(30 * 60 * 1000000);
}

void setup()
{
    dht.begin();
    ccs.begin();
    pinMode(2, OUTPUT);
    Serial.begin(9600); 

    WiFiManager wifiManager;

    digitalWrite(2, LOW);
    Serial.println("Connecting to WiFi");
    delay(500); 
    digitalWrite(2, HIGH);
    delay(500);

    wifiManager.autoConnect();

    Serial.println("Connected to WiFi");
    digitalWrite(2, LOW);

    mainLoop();
} 

void loop()
{
}

void configModeCallback (WiFiManager *wifiManager) {
    digitalWrite(2, HIGH);
}
