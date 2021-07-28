#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <cmath>

#define LAMP_PIN D1
#define BUTTON_PIN D5
#define BUTTON_LED D2

const char *ssid = "RM2100_72B5"; // key in your own SSID "IOT_2518"
const char *password = "1234567890"; // key in your own WiFi access point password "wlwsys6688"
const char *mqttHost = "yandage.top";  // Broker 地址
const char *mqttUsername = "";
const char *mqttPassword = "";
const char *mqttTopic = "/lamp";
const char *clientIDPrefix = "NodeMcu_";

WiFiClient wiFiClient;
MQTTClient mqttClient;

bool isLampOn = false;
bool lampStatus = false;
bool lastButton = false;
unsigned long lastTime = 0;
String clientID(clientIDPrefix);

void connectWiFi() {
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to WiFi ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void messageReceived(String &topic, String &payload) {
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("incoming: " + topic + " - " + payload);

    // Note: Do not use the mqttClient in the callback to publish, subscribe or
    // unsubscribe as it may cause deadlocks when other things arrive while
    // sending and receiving acknowledgments. Instead, change a global variable,
    // or push to a queue and handle it in the loop after calling `mqttClient.loop()`.
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    bool status = doc["status"];
    if (status) {
        isLampOn = true;
    } else {
        isLampOn = false;
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

void connectBroker() {
    Serial.println();
    Serial.print("Connecting to Broker ");
    Serial.println(mqttHost);

    mqttClient.begin(mqttHost, wiFiClient);
    mqttClient.onMessage(messageReceived);
    while (!mqttClient.connect(clientID.c_str(), mqttUsername, mqttPassword)) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Broker connected");
    mqttClient.subscribe(mqttTopic);
    Serial.print("Topic: ");
    Serial.println(mqttTopic);
}

void setLedStatus() {
    static double i = 0.0;
    if (isLampOn) {
        digitalWrite(BUTTON_LED, HIGH);
    } else {
        int val = (int) (abs(sin(i)) * 127);
        analogWrite(BUTTON_LED, val);
        i += 0.001;
        if (i >= 2 * PI) {
            i = 0.0;
        }
    }
}

void setLampStatus() {
    if (isLampOn == lampStatus) return;
    unsigned long nowTime = millis();
//    digitalWrite(LAMP_PIN, isLampOn);

    if (nowTime - lastTime > 2000) {    // 离上次开关后要大于2秒才能开关，保护电灯
        lampStatus = isLampOn;
        digitalWrite(LAMP_PIN, isLampOn);
        lastTime = nowTime;
    }
}

void getButtonStatus() {
    bool buttonStatus = digitalRead(BUTTON_PIN);
    if (buttonStatus != lastButton) {
        lastButton = buttonStatus;
        isLampOn = !buttonStatus;
    }
}

void getFlashButtonStatus() {
    if (!digitalRead(0)) {
        WiFiManager wifiManager;
        wifiManager.resetSettings();
        delay(100);
        EspClass::reset();
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LAMP_PIN, OUTPUT);
    pinMode(BUTTON_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(0, INPUT_PULLUP);
    digitalWrite(LED_BUILTIN, LOW);
    lastButton = !digitalRead(BUTTON_PIN);

    Serial.begin(115200);
    delay(100);

    Serial.println();
    Serial.println();
    clientID += EspClass::getChipId();
    Serial.print("Client ID: ");
    Serial.println(clientID);

    WiFiManager wifiManager;

    bool res;
    res = wifiManager.autoConnect(clientID.c_str()); // password protected ap
    if (!res) {
        Serial.println("Failed to connect");
        EspClass::reset();
    } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
    }
//    connectWiFi();
    connectBroker();
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    mqttClient.loop();
    if (!mqttClient.connected()) {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println();
        Serial.println("Reconnecting");
//        connectWiFi();
        connectBroker();
        if (!mqttClient.connected()) {
            EspClass::reset();
        }
        digitalWrite(LED_BUILTIN, HIGH);
    }

    getFlashButtonStatus();
    getButtonStatus();
    setLampStatus();
    setLedStatus();
}
