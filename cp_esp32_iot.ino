

#include <WiFi.h>
#include <WiFiMulti.h>
#include <PubSubClient.h>
#include "public.h"
#include "wifi_const.h"
#include "mqtt_const.h"



// WIFI_SSID and WIFI_PASSWD are defined in wifi_const.h
//#define WIFI_SSID ""
//#define WIFI_PASSWD ""

// MQTT Broker

//#define MQTT_BROKER "192.168.1.228"
//#define MQTT_PORT 1883

//#define MQTT_USERNAME "admin"
//#define MQTT_PASSWORD "public"

//#define MQTT_TOPIC_PREFIX "CP_IOT/"
//#define MQTT_TOPIC_PING "PING"
//#define MQTT_TOPIC_PONG "PONG"

char DEVICE_UDID[18];



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
  
  Serial.print("\nDevice UUID: ");
  strncpy(DEVICE_UDID, WiFi.macAddress().c_str(), 17);
  Serial.println(DEVICE_UDID);
  
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
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
  while (!mqttClient.connected()) {
      String client_id = "cp-esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (mqttClient.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(mqttClient.state());
          delay(2000);
      }
  }
  // Publish and subscribe
  String mqttPong = String(MQTT_TOPIC_PREFIX) + String(MQTT_TOPIC_PONG);
  Serial.print("publish topic: " + mqttPong);
  mqttClient.publish(mqttPong.c_str(), "{\"udid\": \"420d2b68-6a4a-11ee-8c99-0242ac120002\", \"state\": 1}");
  String mqttPing = String(MQTT_TOPIC_PREFIX) + String(DEVICE_UDID) + "/" + String(MQTT_TOPIC_PING);
  
  Serial.print("\nsubscribe topic: " + mqttPing + "...\n");
  mqttClient.subscribe(mqttPing.c_str());
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
