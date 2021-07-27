// This example uses an Arduino MKR GSM 1400 board
// to connect to shiftr.io.
//
// IMPORTANT: This example uses the new MKRGSM library.
//
// You can check on your device after a successful
// connection here: https://www.shiftr.io/try.
//
// by Sandeep Mistry
// https://github.com/256dpi/arduino-mqtt

#include <MKRGSM.h>
#include <MQTT.h>

const char pin[] = "";
const char apn[] = "apn";
const char login[] = "login";
const char password[] = "password";

GSMClient wiFiClient;
GPRS gprs;
GSM gsmAccess;
MQTTClient mqttClient;

unsigned long lastMillis = 0;

void connect() {
    // connection state
    bool connected = false;

    Serial.print("connecting to cellular network ...");

    // After starting the modem with gsmAccess.begin()
    // attach to the GPRS network with the APN, login and password
    while (!connected) {
        if ((gsmAccess.begin(pin) == GSM_READY) &&
            (gprs.attachGPRS(apn, login, password) == GPRS_READY)) {
            connected = true;
        } else {
            Serial.print(".");
            delay(1000);
        }
    }

    Serial.print("\nconnecting...");
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
