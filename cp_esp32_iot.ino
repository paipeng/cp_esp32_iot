

#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include "public.h"
#include "wifi_const.h"

// WIFI_SSID and WIFI_PASSWD are defined in wifi_const.h
//#define WIFI_SSID ""
//#define WIFI_PASSWD ""

// MQTT Broker
const char *mqtt_broker = "192.168.1.228";
const char *subscribe_topic = "CP_IOT_PING";
const char *publish_topic = "CP_IOT_PONG";
const char *mqtt_username = "admin";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
#define DEVICE_UUID "420d2b68-6a4a-11ee-8c99-0242ac120002"

WiFiMulti WiFiMulti;

// Use WiFiClient class to create TCP connections
WiFiClient client;
PubSubClient mqttClient(client);

//WIFI连接路由器
void wifi_connect(void)
{
  Serial.print("Connecting to ");
  delay(10);

  // We start by connecting to a WiFi network
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWD);

  Serial.println();
  Serial.println();
  Serial.print("\nDefault ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("\n\nWaiting for WiFi... ");

  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
}

void setup(){
  Serial.begin(115200);
  wifi_connect();

  mqtt_connect();
}

void mqtt_connect() {
  Serial.println("Connected to the Wi-Fi network");
  //connecting to a mqtt broker
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(callback);
  while (!mqttClient.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (mqttClient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(mqttClient.state());
          delay(2000);
      }
  }
  // Publish and subscribe
  mqttClient.publish(publish_topic, "{\"deviceUUID\": \"420d2b68-6a4a-11ee-8c99-0242ac120002\", \"state\": 1}");
  mqttClient.subscribe(subscribe_topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}
  
void loop(){
  mqttClient.loop();
}
