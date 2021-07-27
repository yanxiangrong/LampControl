// This example uses an Arduino Uno together with
// an Ethernet Shield to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://www.shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <Ethernet.h>
#include <MQTT.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {192, 168, 1, 177};  // <- change to match your network

EthernetClient wiFiClient;
MQTTClient mqttClient;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("connecting...");
  while (!mqttClient.connect("arduino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  mqttClient.subscribe("/hello");
  // mqttClient.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the mqttClient in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `mqttClient.loop()`.
}

void setup() {
  Serial.begin(115200);
  Ethernet.begin(mac, ip);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  mqttClient.begin("public.cloud.shiftr.io", wiFiClient);
  mqttClient.onMessage(messageReceived);

  connect();
}

void loop() {
  mqttClient.loop();

  if (!mqttClient.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    mqttClient.publish("/hello", "world");
  }
}
