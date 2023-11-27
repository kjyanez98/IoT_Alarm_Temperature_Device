#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

int buzzer = 32;

const char * WIFI_SSID = "Your WIFI_SSID";
const char * WIFI_PASS = "Your WIFI_PASS";

const char * MQTT_BROKER = "Your Broker";
const int MQTT_BROKER_PORT = 8883;

const char * MQTT_CLIENT_ID = "YOUR_CLIENT_ID";
const char * SUBSCRIBE_TOPIC = "Your SUBSCRIBE_TOPIC";
const char * PUBLISH_TOPIC = "Your PUBLISH_TOPIC";

const char * UPDATE_ACCEPTED_TOPIC = "Your UPDATE_ACCEPTED_TOPIC";
const char * UPDATE_TOPIC = "Your UPDATE_TOPIC";


const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
-----END RSA PRIVATE KEY-----
)KEY";

WiFiClientSecure wiFiClient;
PubSubClient mqttClient(wiFiClient);




StaticJsonDocument<JSON_OBJECT_SIZE(3)> outputDocTempShadow;
char outputBufferTempShadow[128];
String temp = "unknown";

void reportTemperature(String str){
  temp = str;
  outputDocTempShadow["state"]["reported"]["temperature"] = temp.c_str();
  serializeJson(outputDocTempShadow, outputBufferTempShadow);
  mqttClient.publish(UPDATE_TOPIC, outputBufferTempShadow);
}

StaticJsonDocument<JSON_OBJECT_SIZE(3)> outputDocShadow;
char outputBufferShadow[128];
String moved = "unknown";

void reportMoved() {
  outputDocShadow["state"]["reported"]["moved"] = moved.c_str();
  serializeJson(outputDocShadow, outputBufferShadow);
  mqttClient.publish(UPDATE_TOPIC, outputBufferShadow);
}

void setMoved(String str) {
  moved = str;
  Serial.println("SetMoved");
  reportMoved();
}



StaticJsonDocument<JSON_OBJECT_SIZE(1)> inputDoc;

void callback(const char * topic, byte * payload, unsigned int lenght) {
  String message;
  for (int i = 0; i < lenght; i++) {
    message += String((char) payload[i]);
  }

  if (String(topic) == SUBSCRIBE_TOPIC) {
    Serial.println("Message from topic " + String(topic) + ":" + message);

    DeserializationError err = deserializeJson(inputDoc, payload);
    if (!err) {
      String state = String(inputDoc["state"].as<const char*>());
      if (state == "moved") {
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(buzzer, HIGH);
      } 
      if(state == "didntmove") {
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(buzzer, LOW);

      }
    }
  }

}

boolean mqttClientConnect() {
  Serial.print("Connecting to " + String(MQTT_BROKER));
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println(" DONE!");

    mqttClient.subscribe(SUBSCRIBE_TOPIC);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC));
    mqttClient.subscribe(UPDATE_ACCEPTED_TOPIC);
    Serial.println("Subscribed to " + String(UPDATE_ACCEPTED_TOPIC));
  } else {
    Serial.println("Can't connect to " + String(MQTT_BROKER));
  }

  return mqttClient.connected();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.begin(115200);
  Serial.print("Connecting to " + String(WIFI_SSID));

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println(" DONE!");

  wiFiClient.setCACert(AMAZON_ROOT_CA1);
  wiFiClient.setCertificate(CERTIFICATE);
  wiFiClient.setPrivateKey(PRIVATE_KEY);

  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  mqttClient.setCallback(callback);


  Serial.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  //setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);	
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  Serial.println("");
  delay(100);

}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;
unsigned long previousPublishTemperatureMillis = 0;

unsigned char counter = 0;

StaticJsonDocument<JSON_OBJECT_SIZE(2)> outputDoc;
char outputBuffer[128];

void publishValueString(String move) {
    outputDoc["moved"] = move;
    serializeJson(outputDoc, outputBuffer);
    mqttClient.publish(PUBLISH_TOPIC, outputBuffer);
}

char movements;

void loop() {
  unsigned long now = millis();
  String tempValue;
  if(mpu.getMotionInterruptStatus()) {
    publishValueString("yes");
    setMoved("yes");
    movements++;
  }
  if (!mqttClient.connected()) {
    if (now - previousConnectMillis >= 2000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) previousConnectMillis = 0;
      else delay(1000);
    }
  } else { // Connected to the MQTT Broker
    mqttClient.loop();
    delay(20);

    if (now - previousPublishMillis >= 20000) {
      previousPublishMillis = now;
      // publish
      publishValueString("no");
      setMoved("no");  
    }

    if(now - previousPublishTemperatureMillis >= 40000){
      previousPublishTemperatureMillis = now;
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      Serial.print(temp.temperature);
      Serial.print("\n");
      tempValue = String(temp.temperature);
      reportTemperature(tempValue);
    }

  }
}