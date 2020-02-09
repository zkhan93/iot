/**
 * Simple server compliant with Mozilla's proposed WoT API
 * Originally based on the HelloServer example
 * Tested on ESP8266, ESP32, Arduino boards with WINC1500 modules (shields or
 * MKR1000)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define LARGE_JSON_BUFFERS 1

#include <Arduino.h>
#include <Thing.h>
#include <WebThingAdapter.h>
#include <ESPAsyncWebServer.h>

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = nullptr;
const char* password = nullptr;

const char* deviceTypes[] = {"OnOffSwitch", nullptr};

ThingDevice device1("device1", "Device Name", deviceTypes);
ThingDevice device2("device2", "Device Name", deviceTypes);
ThingDevice device3("device3", "Device Name", deviceTypes);
ThingDevice device4("device4", "Device Name", deviceTypes);

ThingProperty on1("on", "On/Off", BOOLEAN, "OnOffProperty");
ThingProperty on2("on", "On/Off", BOOLEAN, "OnOffProperty");
ThingProperty on3("on", "On/Off", BOOLEAN, "OnOffProperty");
ThingProperty on4("on", "On/Off", BOOLEAN, "OnOffProperty");

WebThingAdapter* adapter;

bool device1LastOn = false;
bool device2LastOn = false;
bool device3LastOn = false;
bool device4LastOn = false;

AsyncWebServer server(80);
bool wifiConnected = false;
bool shouldReboot = false;

void setup(void){
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  
  // LED lits up
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("");
  Serial.println("Connecting to last working wifi");
  
  // Wait for connection
  int maxAttempt = 10;
  bool blink = false;
  while (WiFi.status() != WL_CONNECTED && maxAttempt > 0) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, blink ? LOW : HIGH); // active low led
    blink = !blink;
    maxAttempt--;
  }
  wifiConnected = WiFi.status() == WL_CONNECTED;
  if (wifiConnected){
    digitalWrite(LED_BUILTIN, LOW); // active low led
  
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  
    adapter = new WebThingAdapter("esp8266", WiFi.localIP());
    device1.addProperty(&on1);
    device2.addProperty(&on2);
    device3.addProperty(&on3);
    device4.addProperty(&on4);
    
    adapter->addDevice(&device1);
    adapter->addDevice(&device2);
    adapter->addDevice(&device3);
    adapter->addDevice(&device4);
    adapter->begin();
    
    Serial.println("HTTP server started");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.print("/things/<id>");
  }else{
    WiFi.mode(WIFI_AP_STA);
    boolean result = WiFi.softAP("ESPsoftAP", "ESPsoftAP");
    if(result == true)
    {
      Serial.println("ESPsoftAP Ready");
      Serial.println(WiFi.softAPIP());
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        
        if(request->hasParam("ssid")){
          AsyncWebParameter* p = request->getParam("ssid");
          ssid = p->value().c_str();
          Serial.println(ssid);
          if(request->hasParam("password")){
            AsyncWebParameter* p = request->getParam("password");
            password = p->value().c_str();
            Serial.println(password);
            shouldReboot = true;
          }else{
            request->send(200, "text/plain", "expecting password GET param");
          }
        }else{
          request->send(200, "text/plain", "expecting ssid GET param");  
        }
        request->send(200, "text/plain", "wifi set");
      });
      server.begin();
      Serial.println("Server Ready");
    }
    else
    {
      Serial.println("ESPsoftAP Failed!");
    }
  }
}

void loop(void){
  if (wifiConnected){
    bool on;

    adapter->update();
    
    on = on1.getValue().boolean;
    digitalWrite(D5, on ? HIGH : LOW); // active low led
    if (on != device1LastOn) {
      Serial.print(device1.id);
      Serial.print(": ");
      Serial.println(on);
    }
    device1LastOn = on;
  
    on = on2.getValue().boolean;
    digitalWrite(D6, on ? HIGH : LOW); // active low led
    if (on != device2LastOn) {
      Serial.print(device2.id);
      Serial.print(": ");
      Serial.println(on);
    }
    device2LastOn = on;  
  
    on = on3.getValue().boolean;
    digitalWrite(D7, on ? HIGH : LOW); // active low led
    if (on != device3LastOn) {
      Serial.print(device3.id);
      Serial.print(": ");
      Serial.println(on);
    }
    device3LastOn = on;
  
    on = on4.getValue().boolean;
    digitalWrite(D8, on ? HIGH : LOW); // active low led
    if (on != device4LastOn) {
      Serial.print(device4.id);
      Serial.print(": ");
      Serial.println(on);
    }
    device4LastOn = on;
  }else{
    if(shouldReboot){
        Serial.println("trying to connect to new WIFI");
        WiFi.begin(ssid, password);
        int tries = 10;
        while (WiFi.status() != WL_CONNECTED && tries > 0){
          Serial.print(".");
          delay(500);
          tries--;
        }
        Serial.println();
        bool isConnected = WiFi.status() == WL_CONNECTED;
        if (isConnected){
          Serial.println("Connected to new WIFI");
          Serial.println("restarting...");
          ESP.restart();
        }else{
          Serial.println("could not connect to new WIFI, try again!");
          shouldReboot = false;
        }
      }
    }
}
