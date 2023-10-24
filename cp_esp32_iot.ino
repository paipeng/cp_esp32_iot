

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

#define BUTTON_PIN 26 // ESP32 GPIO16 pin connected to button's pin


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

  Serial.println("Connected to the Wi-Fi network");
  delay(500);
}

void mqtt_ping() {
  String mqttPong = String(MQTT_TOPIC_PREFIX) + String(MQTT_TOPIC_PONG);
  Serial.print("publish topic: " + mqttPong + "...\n");
  boolean ret = mqttClient.publish(mqttPong.c_str(), "{\"udid\": \"420d2b68-6a4a-11ee-8c99-0242ac120002\", \"state\": 1}");
  if (ret) {
    Serial.print("publish success!\n");
  } else {
    Serial.print("publish error!\n");    
  }
}

void mqtt_subscribe() {
  String mqttPing = String(MQTT_TOPIC_PREFIX) + String(DEVICE_UDID) + "/" + String(MQTT_TOPIC_PING);  
  Serial.print("\nsubscribe topic: " + mqttPing + "...\n");
  boolean ret = mqttClient.subscribe(mqttPing.c_str());
  if (ret) {
    Serial.print("subscribe success!\n");
  } else {
    Serial.print("subscribe error!\n");    
  }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}


void mqtt_connect() {
  //connecting to a mqtt broker
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback);
  mqttClient.setKeepAlive(60*60);
  while (!mqttClient.connected()) {
      String client_id = "cp-esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (mqttClient.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(mqttClient.state());  
          Serial.println("\n");
          delay(2000);
      }
  }    
}

void setup(){
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  
  Serial.begin(115200);
  wifi_connect();

  mqtt_connect();

  // Publish and subscribe
  mqtt_ping();
  mqtt_subscribe();
}

void loop(){
  int buttonState = digitalRead(BUTTON_PIN); // read new state
  if (buttonState == LOW) {
    Serial.println("The button is being pressed");
    mqtt_ping();
  }
  if (mqttClient.connected()) {
    mqttClient.loop();
  } else {
    Serial.println("mqtt disconnected");    
  }
}
