#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
 
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
  JSONencoder["hostname"] = "door";
  JSONencoder["mac"] = ad.getAddress().toString().c_str();
  JSONencoder["rssi"] = ad.getRSSI();;
  //JSONencoder["is_scan_response"] = "1";
  JSONencoder["type"] = "3";
  char JSONmessageBuffer[200];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer); 
  client.loop();
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
 
  Serial.begin(115200);
  Serial.println();
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  setupBLE();
 
}

void reconnect(){
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }  
}

void setupBLE() {
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {

  BLEScanResults scanResults = pBLEScan->start(3); 
  if(!client.connected()){
    reconnect();  
  }
}
