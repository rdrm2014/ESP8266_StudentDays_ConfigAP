#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include <DHT.h>

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <MCP3008.h>

#include "Configuration.h"

MCP3008 _adc  = MCP3008(MCP3008_CLOCK_PIN, MCP3008_MOSI_PIN, MCP3008_MISO_PIN, MCP3008_CS_PIN);
DHT _dht = DHT(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

char msg[50];

char result[200];

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  if(flagArray[0]){
    _dht.begin();
  }
  if(flagArray[5]){
    //MCP3008 _adc = MCP3008(MCP3008_CLOCK_PIN, MCP3008_MOSI_PIN, MCP3008_MISO_PIN, MCP3008_CS_PIN);
  }
  
  
  // put your setup code here, to run once:
  Serial.begin(baudRateSerial);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          
          strcpy(idSystem, json["idSystem"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);

  WiFiManagerParameter custom_idSystem("idSystem", "id System", idSystem, 40);
  
  //WiFiManager
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);

  wifiManager.addParameter(&custom_idSystem);
  
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality - defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off - useful to make it all retry or go to sleep - in seconds
  //wifiManager.setTimeout(120);

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());

  strcpy(idSystem, custom_idSystem.getValue());
  
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;

    json["idSystem"] = idSystem;
  
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //String ssid = "ESP" + String(ESP.getChipId());
  //idESP8266= ssid.c_str();
  idESP8266 = "ESP" + String(ESP.getChipId());

  channelFormat = (String(idSystem) + "/" + idESP8266 + "/%s/%d/%s");
  Serial.print("idESP8266: ");
  Serial.println(idESP8266);
  

  Serial.print("Configuring access point...");

  //pinMode OUTPUT Relay
  for(int i=1; i<=4; i++){
    if(flagArray[i]){
      pinMode(pinArray[i], OUTPUT);
    } 
  }
  
  //pinMode INPUT Hum
  /*if(flagArray[5]){
    pinMode(pinArray[5], INPUT);
  }*/

  //pinMode INPUT Turbidity
  /*if(flagArray[6]){
    pinMode(pinArray[6], INPUT);
  }*/

  setup_MQTT();
}


void loop() {
  
  ArduinoOTA.handle();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //mqttTemp
  if(flagArray[0]){
    mqttTemp();
  }
  
  //mqttHum
  if(flagArray[5]){
    mqttHum();
  }

  //mqttTurbidity
  if(flagArray[6]){
    mqttTurbidity();
  }
}

/**
 * Relay
 * @param      {char*}   result
 */
void Relay(String code, int nRelay) {

  Serial.print("pinArray[nRelay]: ");
  Serial.println(pinArray[nRelay]);
  if (code=="1") {
    digitalWrite (pinArray[nRelay], HIGH);
    snprintf(result, 200, "{\"relay\": 1}");
  } else if (code == "0") {
    digitalWrite (pinArray[nRelay], LOW);
    snprintf(result, 200, "{\"relay\": 0}");
  }
  /*if (code=="1") {
    
    for(int i=1; i<=4; i++){
      channel = String(String(idSystem) + "/" + String(idESP8266) + "/" + "Relay"+ "/" + i + "/" + "read");
      if(flagArray[i] && topic==channel){
        digitalWrite (pinArray[i], HIGH);
        snprintf(result, 200, "{\"relay\": 1}");
      } 
    }
    } else if (code == "0") {
      for(int i=1; i<=4; i++){
      channel = String(String(idSystem) + "/" + String(idESP8266) + "/" + "Relay"+ "/" + i + "/" + "read");
      if(flagArray[i] && topic==channel){
        digitalWrite (pinArray[i], LOW);
        snprintf(result, 200, "{\"relay\": 0}");
      } 
    }
    }*/
     /*   
    if(flagRELAY1 && topic == "ESP8266_Relay1_send") {
      digitalWrite ( pinRELAY1, HIGH);
    } else if(flagRELAY2 && topic == "ESP8266_Relay2_send") {
      digitalWrite ( pinRELAY2, HIGH);
    } else if(flagRELAY3 && topic == "ESP8266_Relay3_send") {
      digitalWrite ( pinRELAY3, HIGH);
    } else if(flagRELAY4 && topic == "ESP8266_Relay4_send") {
      digitalWrite ( pinRELAY4, HIGH);
    }
    
    snprintf(result, 200, "{\"relay\": 1}");
    return result;
  } else if (code == "0") {
    if(flagRELAY1 && topic == "ESP8266_Relay1_send") {
      digitalWrite ( pinRELAY1, LOW);
    } else if(flagRELAY2 && topic == "ESP8266_Relay2_send") {
      digitalWrite ( pinRELAY2, LOW);
    } else if(flagRELAY3 && topic == "ESP8266_Relay3_send") {
      digitalWrite ( pinRELAY3, LOW);
    } else if(flagRELAY4 && topic == "ESP8266_Relay4_send") {
      digitalWrite ( pinRELAY4, LOW);
    }    
    snprintf(result, 200, "{\"relay\": 0}");
    return result;
  }*/
}

/**
 * Temp
 * @param      {char*}   result
 */
void Temp() {  
  float hum = _dht.readHumidity();
  float temp = _dht.readTemperature();  
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (isnan(hum) || isnan(temp) || temp > 80 || temp <-40 || hum <0 || hum >100 ) {
    Serial.print("Result-ERROR: ");
    Serial.println(result);
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  snprintf(result, 200, "{\"hum\": %d.%d, \"temp\": %d.%d}", (int)hum, (int)((hum - (int)hum) * 100), (int)temp, (int)((temp - (int)temp) * 100));
  Serial.println(result);
}

/**
 * Hum
 * @param      {char*}   result
 */
void Hum() { 
  int moistureValue = _adc.readADC(pinArray[5]);
  
  moistureValue = constrain(moistureValue, 250, 750);
  float hum = map(moistureValue, 250, 750, 100, 0);
  
  //Serial.printf("hum: %f", hum);
  snprintf(result, 200, "{\"hum\": %d}", (int)hum);
  Serial.println(result);
}

/**
 * Turbidity
 * @param      {char*}   result
 */
void Turbidity() { 
  int turbidityValue = _adc.readADC(pinArray[6]);
  float voltage = turbidityValue * (5.000 / 1024.000);
  
  //turbidityValue = constrain(turbidityValue, 250, 750);
  float turbidity = 100.00 - (voltage/VClearWater)*100.00;
  
  snprintf(result, 200, "{\"turbidity\": %d}", (int)turbidity);
  Serial.println(result);
}

/************************************ MQTT ************************************/
/**
 * Setup MQTT
 */
void setup_MQTT() {
  
  Serial.print("TESTE: ");
  Serial.print(mqtt_server);
  Serial.print(": ");
  Serial.println(String(mqtt_port).toInt());
  client.setServer(mqtt_server, String(mqtt_port).toInt());
  client.setCallback(callback);
}

/**
 * Callback MQTT
 */
void callback(char* top, byte* payload, unsigned int length) {
  String topic = top;
  String message;

  char topicRead[100];
  
  for (int i = 0; i < length; i++) {    
    message += (char)payload[i];   
  }
  Serial.println(topic);
  Serial.println(message);

  char *serialBuffer[6]; 
  int i=0;
  //char *p = topic;
  char *str;
  while ((str = strtok_r(top, "/", &top)) != NULL){
    Serial.println(str);
    serialBuffer[i++] =str;
  }
    Serial.println(serialBuffer[2]); // VER ERROR!
  if  (String(serialBuffer[2]) == "Relay"){
    Serial.println(serialBuffer[3]);
    int nRelay = atoi(serialBuffer[3]);
    Serial.print("nRelay");
    Serial.println(nRelay);
    if  (flagArray[nRelay]){
       mqttRelay(message, nRelay); 
    }
  }
  //Serial.println(serialBuffer[2]);
  //Serial.println(serialBuffer[3]);
  
  /*
  for(int i=1; i<=4; i++){
    sprintf(topicRead, channelFormat.c_str(), "Relay", i, "write");
    if  (flagArray[i] && topic == topicRead){
      sprintf(topicRead, channelFormat.c_str(), "Relay", i, "read");
      mqttRelay(message, topicRead); 
    }
  }*/
}

/**
 * Reconnect
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(idESP8266.c_str())) {
      Serial.println("connected");
    
      //client.subscribe Relay
      for(int i=1; i<=4; i++){
        if(flagArray[i]){
          //channel = String(String(idSystem) + "/" + idESP8266 + "/" + "Relay"+ "/" + i + "/" + "write");

          sprintf(channel_char, channelFormat.c_str(), "Relay", i, "write");
          Serial.println(channel_char);
          //client.subscribe(channel);
          client.subscribe(channel_char);
        } 
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
 * mqttRelay
 */
void mqttRelay(String code, int nRelay) {
  Relay(code, nRelay);
  /*sprintf(topicRead, channelFormat.c_str(), "Relay", i, "write");
  client.publish(topic.c_str(), result);*/
  sprintf(channel_char, channelFormat.c_str(), "Relay", nRelay, "read");
  //Serial.println(channel_char);
  client.publish(channel_char, result);
}

/**
 * mqttTemp
 */
void mqttTemp() {
  long now = millis();
  if (now - lastUpdateTimeArray[0] > updateTimeArray[0]) {
    lastUpdateTimeArray[0] = now;
    Temp();     
    sprintf(channel_char, channelFormat.c_str(), "DHT", 1, "read");
    //Serial.print("channel_char: ");
    //Serial.print(String(channel_char));
    //Serial.println(String(result));
    //channel = String(String(idSystem) + "/" + String(idESP8266) + "/" + "DHT"+ "/" + 1 + "/" + "read");
    client.publish(channel_char, result);
  }
}

/**
 * mqttHum
 */
void mqttHum() {
  long now = millis();
  if (now - lastUpdateTimeArray[5] > updateTimeArray[5]) {
    lastUpdateTimeArray[5] = now;
    Hum();
    //channel = String(String(idSystem) + "/" + String(idESP8266) + "/" + "hum"+ "/" + 1 + "/" + "read");
    sprintf(channel_char, channelFormat.c_str(), "hum", 1, "read");
    //Serial.println(channel_char);
    client.publish(channel_char, result);
  }
}

/**
 * mqttTurbidity
 */
void mqttTurbidity() {
  long now = millis();
  if (now - lastUpdateTimeArray[5] > updateTimeArray[5]) {
    lastUpdateTimeArray[5] = now;
    Turbidity();
    //channel = String(String(idSystem) + "/" + String(idESP8266) + "/" + "hum"+ "/" + 1 + "/" + "read");
    sprintf(channel_char, channelFormat.c_str(), "turbidity", 1, "read");
    //Serial.println(channel_char);
    client.publish(channel_char, result);
  }
}
