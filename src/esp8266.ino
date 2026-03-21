#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// WiFi credentials
const char* ssid = "Home";
const char* password = "24102005";

// MQTT broker
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

// Servo objects
Servo servo1, servo2, servo3, servo4, servo5, servo6;

// NodeMCU pin assignments
const int servo_pins[6] = {D3, D4, D5, D0, D1, D2};
Servo* servos[6] = {&servo1, &servo2, &servo3, &servo4, &servo5, &servo6};

// MQTT topics
const char* topics[6] = {
    "robotic_arm/servo1",
    "robotic_arm/servo2",
    "robotic_arm/servo3",
    "robotic_arm/servo4",
    "robotic_arm/servo5",
    "robotic_arm/servo6"
};

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    Serial.begin(115200);
    
    // Attach each servo to its respective pin
    for (int i = 0; i < 6; i++) {
        servos[i]->attach(servo_pins[i]);
    }

    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // Connect to MQTT broker
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    reconnect();
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("NodeMCU_RobotArm")) {
            Serial.println("connected");
            for (int i = 0; i < 6; i++) {
                client.subscribe(topics[i]);
            }
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0'; // Null-terminate payload
    int value = atoi((char*)payload); // Convert payload to int
    value = constrain(value, 0, 180);

    for (int i = 0; i < 6; i++) {
        if (strcmp(topic, topics[i]) == 0) {
            Serial.printf("Servo %d set to %d\n", i + 1, value);
            servos[i]->write(value);
            break;
        }
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}