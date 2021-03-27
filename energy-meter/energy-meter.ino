/*
 * Nov 30, 2020, Binh Nguyen
 * PZEM 004T Power meter
 * with data publish to MQTT server
 *
 */

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);
PZEM004Tv30 pzem(D6, D5); //RX-TX

/*_____________________WIFI and MQTT_______________________*/
#define wifi_ssid "MyWiFi"            // change this
#define wifi_password "WiFiPass"      // change this
#define mqtt_server "192.168.1.66"    // change this
#define mqtt_user "mqtt_user"         // change this
#define mqtt_password "mqtt_pass"     // change this
#define mqtt_port 1883
#define publish_topic "sensors/power"

#define OTApassword "use_update_sketch"
#define SENSORNAME "main_power"
char HOSTNAME[30];

uint8_t INVL = 30; //30 second
uint32_t lastPush=0;


void setup(){
  Serial.begin(115200);
  Serial.println("Starting");
  
  setup_wifi();
  mqtt.setServer(mqtt_server, mqtt_port);
  setup_OTA();
  delay(2000);
}

void loop(){
  // main program here
  ArduinoOTA.handle();
  uint32_t now_ = millis()/1000;
  if ((now_ -lastPush)>=INVL){
    DynamicJsonDocument doc(1024);
    doc["sensor"] = SENSORNAME;
    doc["type"] = "json"; //format of message
    doc["uptime"] = now_;
    readPZEM(doc);
    pushData(doc);
    lastPush = now_;
    JsonObject obj = doc.to<JsonObject>();
    
    if (now_ > 86400L){
      ESP.restart();
    }
  }
  delay(1000);
}

void readPZEM(DynamicJsonDocument &doc){
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();
    doc["V"] = voltage;
    doc["A"] = current;
    doc["W"] = power;
    doc["kWh"] = energy;
    doc["Hz"] = frequency;
    doc["PF"] = pf;
}

/*______________    SETUP WIFI     _______________*/

void create_host_name(){
  char MAC[6];
  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);
  sprintf(MAC, "%02x%02x%02x", macAddr[3], macAddr[4], macAddr[5]);
//  Serial.println(MAC);
  String NAME_TMP;
  NAME_TMP = String(SENSORNAME) + "__" + String(MAC);
  NAME_TMP.trim();
  byte str_len = NAME_TMP.length() + 1; 
  NAME_TMP.toCharArray(HOSTNAME, str_len);
  Serial.printf("Hostname: %s\n", HOSTNAME);
}

void setup_wifi() {
  delay(10);
  Serial.printf("Connecting to %s", wifi_ssid);
  WiFi.mode(WIFI_STA);
  create_host_name();
  WiFi.hostname(HOSTNAME);
  WiFi.begin(wifi_ssid, wifi_password);
  delay(100); 
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    i++;
    Serial.printf(" %i ", i);
    if (i == 5){
      WiFi.mode(WIFI_STA);
      WiFi.begin(wifi_ssid, wifi_password);
      delay(2000);
    }
    if (i >=10){
      ESP.restart();
      Serial.println("Resetting ESP");
    }
  }
  Serial.printf("\nWiFi connected: \t");
  Serial.print(WiFi.localIP());
  Serial.print("\twith MAC:\t");
  Serial.println(WiFi.macAddress());
  
}
/*__________________ PUSH DATA  ___________________*/
void pushData(DynamicJsonDocument &doc) {
      size_t len = measureJson(doc)+ 1;
    char payload[len];
    serializeJson(doc, payload, sizeof(payload));
    
   if (!mqtt.connected()) {
    reconnect();
    delay(1000);
  }
  if (mqtt.publish(publish_topic, payload, false)){
    Serial.println("Success: " + String(payload));
  } else {
    Serial.println("Failed to push: " + String(payload));
  }
}

/*_____________________START RECONNECT_______________________*/
void reconnect() {
  while (WiFi.status() != WL_CONNECTED){
  setup_wifi();
  }
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(SENSORNAME, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/*_____________________START OT_______________________*/
void setup_OTA(){
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword((const char *)OTApassword);
  ArduinoOTA.onStart([]() {
    Serial.println("Starting");
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
}
