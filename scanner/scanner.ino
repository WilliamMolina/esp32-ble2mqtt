#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "esp_pm.h"
#include "esp32/pm.h"
#include "esp32-hal-cpu.h"

const char* ssid = "******";
const char* password =  "********";
const char* mqttServer = "xxx.xxx.xxx.xxx";
const int mqttPort = 1883;
const char* mqttUser = "*******";
const char* mqttPassword = "******";
BLEScan* pBLEScan;
BLEClient* pClient;
 
WiFiClient espClient;
PubSubClient client(espClient);

void sendMessage(BLEAdvertisedDevice ad){
  String topic = "happy-bubbles/ble/living-room/raw/";
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject(); 
  JSONencoder["hostname"] = "home";
  JSONencoder["mac"] = ad.getAddress().toString().c_str();
  JSONencoder["rssi"] = ad.getRSSI();;
  JSONencoder["type"] = "3";
  char JSONmessageBuffer[200];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer); 
  //client.loop();
  Serial.println("-------------");
  topic.concat(ad.getAddress().toString().c_str());
  if (client.publish(topic.c_str(), JSONmessageBuffer) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
    Serial.println(client.state());
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice ad){
    Serial.println("BLE Advertised device found:");
    Serial.println(ad.getRSSI());  
    sendMessage(ad);
  }  
};
void setup() { 
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  Serial.println("Starting BLE to MQTT scanner");
  Serial.print("Frecuencia: ");
  Serial.println(getCpuFrequencyMhz());
  connect_wifi(); 
  client.setServer(mqttServer, mqttPort);
  connect_mqtt();
  setup_ble(); 
}

void connect_mqtt(){
  while (!client.connected()) {
    Serial.println("Connecting to MQTT..."); 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) { 
      Serial.println("connected"); 
    } else { 
      Serial.print("failed with state ");
      Serial.println(client.state());
    }
  }  
}

void connect_wifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  } 
  Serial.println(WiFi.localIP());
  Serial.println("Connected to the WiFi network");
}
void setup_ble() {
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {
  //TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  //TIMERG0.wdt_feed=1;
  //TIMERG0.wdt_wprotect=0;
  BLEScanResults scanResults = pBLEScan->start(31);
  //esp_sleep_enable_timer_wakeup(60 * 1000000);
  //esp_deep_sleep_start();
  rtc_cpu_freq_t freq = rtc_clk_cpu_freq_get();
  Serial.print("Frecuencia: ");
  Serial.println(freq);
  Serial.println("Scanning...");
  //delay(3000);
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Retrying to connect WiFi..");
    connect_wifi();
  }  
  if(!client.connected()){
    Serial.println("Retrying to connect to MQTT Server..");
    connect_mqtt();  
  }
}
