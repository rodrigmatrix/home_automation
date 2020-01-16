#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h> 
#include <ArduinoJson.h> 
#include <StreamString.h>

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

const int ledPin = 2;
#define MyApiKey "027a4817-08eb-4864-9f62-e801fc0ee0a7"
#define MySSID "Dunder Mifflin Paper"
#define MyWifiPassword "54321111"

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

 
void turnOn(String deviceId) {
  if (deviceId == "5e1fad540c04793a3a7f9e20")
  {  
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    digitalWrite (ledPin, HIGH);
  } 
  else if (deviceId == "5e1fad540c04793a3a7f9e20") 
  { 
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
  }
  else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }     
}

void turnOff(String deviceId) {
   if (deviceId == "5e1fad540c04793a3a7f9e20")
   {  
     Serial.print("Turn off Device ID: ");
     Serial.println(deviceId);
     digitalWrite (ledPin, LOW);
   }
   else if (deviceId == "5e1fad540c04793a3a7f9e20")
   { 
     Serial.print("Turn off Device ID: ");
     Serial.println(deviceId);
  }
  else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(deviceId);    
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
      
#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);      
#endif        
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "action.devices.commands.OnOff") { // Switch 
            String value = json ["value"]["on"];
            Serial.println(value); 
            
            if(value == "true") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if(action == "action.devices.commands.BrightnessAbsolute"){
          String brightnessLevel = json ["value"]["brightness"];
          Serial.println(brightnessLevel);
          turnOn(deviceId);
        }
        else if(action == "action.devices.commands.ColorAbsolute"){
          String color = json ["value"]["color"]["spectrumRGB"];
          Serial.println(color);
          turnOn(deviceId);
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
        
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    default: break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode (ledPin, OUTPUT);
  Serial.println("Connecting to Wifi: ");
  Serial.println("Connected to the WiFi network");
  
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.println("Connecting to Wifi: ");
  Serial.println(MySSID);

  // Waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  webSocket.begin("iot.sinric.com", 80, "/");

  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();  
  if(isConnected) {
      uint64_t now = millis(); 
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }
}
 
