
// WIFI
#include <WiFi.h>
#include <WiFiMulti.h>
// MQTT
#include <PubSubClient.h>
// Temeprature
#include <DallasTemperature.h>
// oled
#include <SSD1306Wire.h>

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


// OLED

int counter = 1;

//OLED引脚定义
#define SDA   23
#define SCL   18
SSD1306Wire display(0x3c, SDA, SCL);


// LED
#define CP_GPIO_LED    15 //1-wire数据总线连接



// temperature
#define ONE_WIRE_BUS    13 //1-wire数据总线连接
OneWire oneWire(ONE_WIRE_BUS); //声明
DallasTemperature sensors(&oneWire); //声明


// button
#define BUTTON_PIN 26 // ESP32 GPIO16 pin connected to button's pin
#define BUTTON2_PIN 27
#define BUTTON3_PIN 25


char DEVICE_UDID[18];
int LED_STATE = 0;


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
  
  Serial.print("\nDevice UDID: ");
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

void mqtt_pong() {
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
  // udid ping
  String mqttPing = String(MQTT_TOPIC_PREFIX) + String(DEVICE_UDID) + "/" + String(MQTT_TOPIC_PING);  
  Serial.print("\nsubscribe udid ping topic: " + mqttPing + "...\n");
  boolean ret = mqttClient.subscribe(mqttPing.c_str());
  if (ret) {
    Serial.print("subscribe udid ping success!\n");
  } else {
    Serial.print("subscribe udid ping error!\n");    
  }
  // broadcasting ping
  mqttPing = String(MQTT_TOPIC_PREFIX) + String(MQTT_TOPIC_PING);  
  Serial.print("\nsubscribe broadcasting ping topic: " + mqttPing + "...\n");
  ret = mqttClient.subscribe(mqttPing.c_str());
  if (ret) {
    Serial.print("subscribe broadcasting ping success!\n");
  } else {
    Serial.print("subscribe broadcasting ping error!\n");    
  }
  // LED
  mqttPing = String(MQTT_TOPIC_PREFIX) + String(DEVICE_UDID) + "/" + String(MQTT_TOPIC_LED);  
  Serial.print("\nsubscribe led topic: " + mqttPing + "...\n");
  ret = mqttClient.subscribe(mqttPing.c_str());
  if (ret) {
    Serial.print("subscribe led success!\n");
  } else {
    Serial.print("subscribe led error!\n");    
  }
}

void gpio_led_toggle(int state) {
  Serial.println("gpio_led_toggle: " + String(state));
  LED_STATE = state;
  if (state == 1) {
    digitalWrite(CP_GPIO_LED, HIGH);//LED1引脚输出高电平，点亮
  } else {
    digitalWrite(CP_GPIO_LED, LOW);//LED1引脚输出低电平，熄灭
  }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("-----------------------mqtt arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");

    String data;
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
        data += (char) payload[i];
    }
    Serial.println();
    
    Serial.println("Message len:" + length);
    Serial.println("data: " + data);

    if (strstr(topic, "PING")) {
      mqtt_pong();
      
    } else if (strstr(topic, "LED")) {
      if (data.equals("1")) {
        gpio_led_toggle(1);
      } else {
        gpio_led_toggle(0);
      }
    }
    Serial.println();
    Serial.println("-----------------------mqtt-end");
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

void mqtt_publish_temperature(float temperature) {
  String mqttTopic = String(MQTT_TOPIC_PREFIX) + String(MQTT_TOPIC_TEMPERATURE);
  Serial.print("publish topic: " + mqttTopic + "...\n");
  String data = "{\"udid\": \"" + String(DEVICE_UDID) + "\", \"state\": 1, \"value\": " + temperature + "}";
  boolean ret = mqttClient.publish(mqttTopic.c_str(), data.c_str());
  if (ret) {
    Serial.print("publish temperature success!\n");
  } else {
    Serial.print("publish temperature error!\n");    
  }
}

float gpio_read_temperature() {
  Serial.println("发起温度转换");
  sensors.requestTemperatures(); //向总线上所有设备发送温度转换请求，默认情况下该方法会阻塞
  Serial.println("温度转换完成");

  float tempC = sensors.getTempCByIndex(0); //获取索引号0的传感器摄氏温度数据
  if (tempC != DEVICE_DISCONNECTED_C) //如果获取到的温度正常
  {
    Serial.print("当前温度是： ");
    Serial.print(tempC);
    Serial.println(" ℃\n");
  } else {
    Serial.println("read temperature error\n");
  }
  return tempC;
}

void oled_init() {
  display.init();//初始化UI
  display.flipScreenVertically();//垂直翻转屏幕设置
}

void oled_draw_title() {
  display.setFont(ArialMT_Plain_24);//设置字体大小
  display.drawString(0, 0, "CP IOT");//显示
  display.display();//将缓存数据写入到显示器
}

void setup(){
  delay(2000);
  Serial.begin(115200);

  pinMode(CP_GPIO_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  sensors.begin();
  
  oled_init();
  oled_draw_title();

  
  wifi_connect();

  mqtt_connect();

  // Publish and subscribe
  mqtt_pong();
  mqtt_subscribe();
}

void loop(){
  int buttonState = digitalRead(BUTTON_PIN); // read new state
  if (buttonState == LOW) {
    Serial.println("The button is being pressed");
    mqtt_pong();
    delay(1000);
  }
  buttonState = digitalRead(BUTTON2_PIN);
  if (buttonState == LOW) {
    Serial.println("The button2 is being pressed");
    float temperature = gpio_read_temperature();
    if (temperature != DEVICE_DISCONNECTED_C) {
      String temp = "Temp: " + String(temperature) + " °C";
      display.clear();
      oled_draw_title();
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 40, temp);
      display.display();//将缓存数据写入到显示器
      mqtt_publish_temperature(temperature);
    }
    delay(1000);
  }
  buttonState = digitalRead(BUTTON3_PIN);
  if (buttonState == LOW) {
    Serial.println("The button3 is being pressed");
    
    
    if (LED_STATE == 0) {
      LED_STATE = 1;
    } else {
      LED_STATE = 0;
    }

    gpio_led_toggle(LED_STATE);
    delay(1000);
  }
  if (mqttClient.connected()) {
    mqttClient.loop();
  } else {
    Serial.println("mqtt disconnected");    
  }
}
