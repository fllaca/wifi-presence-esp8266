#include "ConfigManager.h"
#include <FS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>

struct Config {
    char name[20];
    bool enabled;
    int8_t hour;
    char password[20];
    char ifttt_event[32];
    char ifttt_key[32];
    char ifttt_fingerprint[64]; //"AA:75:CB:41:2E:D5:F9:97:FF:5D:A0:8B:7D:AC:12:21:08:4B:00:8C"
} config;

struct Metadata {
    int8_t version;
} meta;

ConfigManager configManager;
ESP8266HTTPUpdateServer httpUpdater;
bool sent = false;

void createCustomRoute(WebServer *server) {
    // Create Upload endpoint
    // Upload firmware with: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
    httpUpdater.setup(server);
    
    server->on("/ifttt", HTTPMethod::HTTP_GET, [server](){
      //server->send(200, "text/plain", "Hello, World!");
      SPIFFS.begin();

      File f = SPIFFS.open("/ifttt.html", "r");
      if (!f) {
          Serial.println(F("file open failed"));
          server->send(404, FPSTR("text/html"), F("File not found"));
          return;
      }

      server->streamFile(f, FPSTR("text/html"));

      f.close();
    });
}

bool sendIFTTTTrigger(char ifttt_event[32], char ifttt_key[32], char ifttt_fingerprint[64], int retries)
{

  int httpCode = 0;
  int attempt = 0;
  while (httpCode != 200 && attempt < retries) {
    HTTPClient http; 
    http.begin(
      String("https://maker.ifttt.com/trigger/") + ifttt_event +  "/with/key/" + ifttt_key,
      String(ifttt_fingerprint)); 
    http.addHeader("Content-Type", "application/json"); 

    httpCode = http.POST("{}"); //Send the request
    String payload = http.getString(); //Get the response payload

    Serial.println(httpCode); //Print HTTP return code
    Serial.println(payload);  //Print request response payload

    http.end(); //Close connection
    attempt++;
    delay(500);
  }
  return httpCode == 200;
  
}

void setup() {
    Serial.begin(9200);
    Serial.println("");

    meta.version = 3;

    // Setup config manager
    configManager.setAPName("Demo");
    configManager.setAPFilename("/index.html");
    configManager.addParameter("name", config.name, 20);
    configManager.addParameter("enabled", &config.enabled);
    configManager.addParameter("hour", &config.hour);
    configManager.addParameter("password", config.password, 20, set);
    configManager.addParameter("version", &meta.version, get);
    configManager.addParameter("ifttt_event", config.ifttt_event, 32);
    configManager.addParameter("ifttt_key", config.ifttt_key, 32);
    configManager.addParameter("ifttt_fingerprint", config.ifttt_fingerprint, 64);

    configManager.setAPCallback(createCustomRoute);
    configManager.setAPICallback(createCustomRoute);
    configManager.setForceSavedWifi(true);

    configManager.begin(config);
}

void loop() {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      configManager.loop();

      if (!sent)
      {
        sent = sendIFTTTTrigger(
           config.ifttt_event, config.ifttt_key, 
           config.ifttt_fingerprint, 5);

        Serial.println("Sent IFFF Trigger");
      }
      
    }
    else
    {
      //Serial.println("Error in WiFi connection");
      sent = false;
    }
}
