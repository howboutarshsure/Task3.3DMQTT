#include <PubSubClient.h>
#include <WiFiNINA.h>

// MQTT Broker configuration
const char* mqttHost = "mqtt-dashboard.com";
const char* publishTopic = "SIT210/wave";
const int mqttPort = 1883;

// Pins for Ultrasonic Sensor
const int triggerPin = 9;
const int echoPin = 10;

// Pin for the status LED
const int statusLedPin = 13;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  setupWiFi();
  mqttClient.setServer(mqttHost, mqttPort);
  mqttClient.setCallback(messageReceived);

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(statusLedPin, OUTPUT);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  long timeEcho, calcDistance;
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  timeEcho = pulseIn(echoPin, HIGH);
  calcDistance = timeEcho * 0.034 / 2;

  if (calcDistance < 30) {
    mqttClient.publish(publishTopic, "Pat detected!");
  }

  if (calcDistance >= 30 && calcDistance < 100) {
    mqttClient.publish(publishTopic, "Wave detected!");
  }

  delay(2000); // Pause between sensor readings
}

void setupWiFi() {
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Connecting to WiFi SSID: ");
    Serial.println("MINEZ_2G");
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin("MINEZ_2G", "69699696");
      Serial.print(".");
      delay(5000);     
    }
  }
  Serial.println("\nWiFi connection established!");
}

void messageReceived(char* topic, byte* message, unsigned int length) {
  Serial.print("Received message on topic: ");
  String receivedMsg;
  for (int i = 0; i < length; i++) {
    receivedMsg += (char)message[i];
  }
  Serial.println(receivedMsg);

  if (receivedMsg.indexOf("Wave") > -1) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(statusLedPin, HIGH);
      delay(100);
      digitalWrite(statusLedPin, LOW);
      delay(100);
    }
  } else if (receivedMsg.indexOf("Pat") > -1) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(statusLedPin, HIGH);
      delay(500);
      digitalWrite(statusLedPin, LOW);
      delay(500);
    }
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Trying to connect to MQTT...");
    if (mqttClient.connect("Arduino33IoT")) {
      Serial.println("MQTT connected");
      mqttClient.subscribe(publishTopic);
    } else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println("; retrying in 5 seconds");
      delay(5000);
    }
  }
}
