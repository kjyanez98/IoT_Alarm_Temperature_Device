#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

int buzzer = 32;

const char * WIFI_SSID = "FAMILIA MAMANI";
const char * WIFI_PASS = "0131BD22019";

const char * MQTT_BROKER = "ag6oftoa7eyp-ats.iot.us-east-2.amazonaws.com";
const int MQTT_BROKER_PORT = 8883;

const char * MQTT_CLIENT_ID = "YOUR_CLIENT_ID";
const char * SUBSCRIBE_TOPIC = "iot/topic/pub";
const char * PUBLISH_TOPIC = "iot/topic/sub";

const char * UPDATE_ACCEPTED_TOPIC = "$aws/things/Mything2/shadow/update/accepted";
const char * UPDATE_TOPIC = "$aws/things/Mything2/shadow/update";

bool estado = false;

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
MIIDWTCCAkGgAwIBAgIUJYnzcocZkZj0FyhGewNO2+BduF8wDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIzMTAzMDE4NTYw
OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKyJIWabrmx00pFIFCv+
Kge1GmYDoeuzCUtMWRCjREVtynFpn9gpOHJBUJwtgVptjCISbiRwj9xEMFmg7sla
wXm5B6rDRf1CC+GR8J+wSqV2nSv6ZKkP4nD6P434iH3JPZTmWaTHsABN78SWHAng
89EuiwqIYjNvLKvJVIlHTFxJhPEQwgCDyaVVfdDs2ispEpopu90zCmpfW1wucix8
bX9GGZkdIq2x0MvEZnDO9yEYtxsIwaUBZJFTOed1n0kW7Vps9lmdG9QkBvC6hBaN
ET6yImwLFRF5CfprPLzZFlYV659NRt6I/YOXjy6CT6zxN5Xw+l0AvBB3AAmuTve2
3pkCAwEAAaNgMF4wHwYDVR0jBBgwFoAUH3eUe5PSjBD5EkCmw0oEOtFH81MwHQYD
VR0OBBYEFA7V/YekA8Qw8zroNTqwSzMdWxNeMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCE4/CulIdVzRM9pNf9d3p3nXHw
R7nqUwDQbQOeMvbC3/XlMkuCdxxnYogeZCUFpbWzXiLf0G774B1u+o7v4GGWXrp4
9A9gSTKz8z3V+LlKBfhBIU+7WuNw5/hBpOvettIa5VFKRDWPeHSCHulnpT9hk4Hv
kVQmExXiQ1SnEdaTCx1AbGivrVEsXhD2KI8HN0r7NH91kr6pibEOX+tlwQjXrBje
Swzf1O8FCmJoznSwVhAqGKC3SnMjWjmVSHuRzqf+Q8x6osIco+tSuc4/FQOXSrv5
teyVUh5QqM6A1+cq2jJ5BHH1Oe2aeutjTTgo5vbiz2CVCy09grbKgF0olCa4
-----END CERTIFICATE-----

)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEArIkhZpuubHTSkUgUK/4qB7UaZgOh67MJS0xZEKNERW3KcWmf
2Ck4ckFQnC2BWm2MIhJuJHCP3EQwWaDuyVrBebkHqsNF/UIL4ZHwn7BKpXadK/pk
qQ/icPo/jfiIfck9lOZZpMewAE3vxJYcCeDz0S6LCohiM28sq8lUiUdMXEmE8RDC
AIPJpVV90OzaKykSmim73TMKal9bXC5yLHxtf0YZmR0irbHQy8RmcM73IRi3GwjB
pQFkkVM553WfSRbtWmz2WZ0b1CQG8LqEFo0RPrIibAsVEXkJ+ms8vNkWVhXrn01G
3oj9g5ePLoJPrPE3lfD6XQC8EHcACa5O97bemQIDAQABAoIBABRic0bLN/VqbAJs
MK3t7otQk1jgLv5I/d0enRtBHuzRBu0VXKROYnPlUZ/SJIlJjhfsc4Tso6KTj97X
C79QcLFv8f7z6U39wqeY9YP4cSlRdUpf4Aq1bLyKZH3ikkXCQ/tWp9q3GQ1AKAbi
8Vcdc9EQHa9V47xN80oslEdP3C6+80OwcYukMP8LzW3hea+12GwAKwEW6S9fPuVL
Ydt/xSF2lVLu05eVxEVLsqBC4pXEEQ2gbJ1M3wZBDSxuOM/uQhaeBEKDEsqTUxwh
DrLg5LuPawNXqbMj5/fj3kvbUExtf0fd2mFfGsJhHS3+jnJgGoK0/3EY9G65AwIT
IeVfAAECgYEA0nqkC1vU+BIh5HDHixVRmhE7BtRa/Mw2hHj+MWKUhbdO2+QpNw4P
RVt9tInXdvTNKFj81enV9Agy/7GFaBpKnBrS4eFQvMWCLEXlMuMmnohflfVIIijF
4RAi5TbZBWjt4YTAV0JIzaxBxxVhODhutPNLhurbwSq1b4vOnB3cAAECgYEA0dm4
SSOEk8YoLU5UErUu+13mLU5X5TTgIH+6exTFVawBMySZPyhvI0p0QdKuzVn/nrQ+
boizadHkoqNT7EJNbKDEmhbrW2YcXBcsHGcW+sfoEdgsWXlkiNXKGsOsi8s1Vw9v
7q1cYnF5kcYFEsutKgj9ZCsDxKc4kFtJRFc63pkCgYB+eEintpdCNAr/a7DK5pVw
wNe549GEQuSjNtxuKjDaI+oTgIYvWZhdcXsYBxdbl/7KNqY9ltMDXgXe0/k4M0wP
fqKmLS/JLdho59qveAPVkFX3Ejo0pFE6Serd1dYTRRhiwNSd2etLAi1IBZW0JVyI
CASi3e4LpLD+hIAyGFMAAQKBgGwu67x43Q45p9jDesAyO8hGvjU5IEL/oVPfxUlS
ifECvZOaGI7gz3rIFWshrBAdh5RPo6fkPWoeVNorjUZNYBDSSMiPbEIwQAqRv6Hc
25HpoRAh+6sc0+FfJdPqcHeZCSSaxr+rC+eMTHoENfVcnyb+f3M2Ybjt5FE3Xmwm
9ChxAoGADFEotb/aWoT2YoyNcga4O0WzgxsyJ0S2ybzP0t5ob686gc8AZKvMdRg3
TAVajr4HVzzlxvqi3axBbTdruTwzX1cYIG0eVWEZcLaar3MysssI7r1oSD3q1Gfn
nvxIiPxpw3keo2ihyQTRJW2MSPUUMHk813SsyiW/GqW2LxnAK9M=
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

  // if (String(topic) == UPDATE_ACCEPTED_TOPIC) {
  //   Serial.println("Message from topic " + String(topic) + ":" + message);

  //   DeserializationError err = deserializeJson(inputDoc, payload);
  //   if (!err) {
  //     String tmpMoved = String(inputDoc["state"]["reported"]["moved"].as<const char*>());
  //     if(!tmpMoved.isEmpty() && !tmpMoved.equals(moved))
  //     {
  //       setMoved(tmpMoved);
  //     }
       
  //   }
  // }

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

  // TwoWire myWire = TwoWire(0);
  // myWire.begin(2,4);

  // Try to initialize!
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
  mpu.setInterruptPinLatch(true);	// Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  Serial.println("");
  delay(100);
  
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;
unsigned long previousPublishTemperatureMillis = 0;

unsigned char counter = 0;
// String move;

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
