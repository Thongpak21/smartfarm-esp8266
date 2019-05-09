#include <Arduino.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#define FIREBASE_HOST "smartfarm-50309.firebaseio.com"
#define FIREBASE_AUTH "E8Mq53C112QdluHI32ANRaZyVXsIlLFvsIpDVat4"
#define WIFI_SSID "Nextzy"
#define WIFI_PASSWORD "eyeshield"
unsigned long sendDataPrevMillis = 0;

String path = "/users/qweasdzxc";

uint16_t count = 0;
FirebaseData firebaseData;
void streamCallbackData(StreamData data)
{

  //Print out all information

  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  //Print out value
  //Stream data can be many types which can be determined from function dataType

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    Serial.println(data.doubleData(), 9);
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
    Serial.println(data.jsonData());
}

//Global function that notify when stream connection lost
//The library will resume the stream connection automatically
void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    //Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }
}
void setupWifi()
{
  Serial.println(WiFi.localIP());
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    digitalWrite(2, !digitalRead(2));
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setMaxRetry(firebaseData, 3);
  String path = "/users/qweasdzxc";
  String jsonStr;
  Serial.println("----------");
  Serial.println("path exist test...");
  if (Firebase.pathExist(firebaseData, path))
  {
    Serial.println("Path" + path + "exists");
  }
  else
  {
    Serial.println("Path" + path + " not exists");
  }

  Serial.println("------------------------------------");
  if (!Firebase.beginStream(firebaseData, path))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}
void setup()
{
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(D1, OUTPUT);
  setupWifi();
}

void loop()
{
  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't read stream data...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

  if (firebaseData.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + firebaseData.streamPath());
    Serial.println("EVENT PATH: " + firebaseData.dataPath());
    Serial.println("DATA TYPE: " + firebaseData.dataType());
    Serial.println("EVENT TYPE: " + firebaseData.eventType());
    Serial.print("VALUE: ");
    if (firebaseData.dataType() == "int")
      Serial.println(firebaseData.intData());
    else if (firebaseData.dataType() == "float")
      Serial.println(firebaseData.floatData());
    else if (firebaseData.dataType() == "boolean")
      Serial.println(firebaseData.boolData() == 1 ? "true" : "false");
    else if (firebaseData.dataType() == "string")
      Serial.println(firebaseData.stringData());
    else if (firebaseData.dataType() == "json")
    {
      StaticJsonDocument<200> doc;
      deserializeJson(doc, firebaseData.jsonData());
      bool isWaterOn = doc["isWaterOn"];
      digitalWrite(D1, !isWaterOn);
      Serial.println(isWaterOn);
    }

    Serial.println("------------------------------------");
    Serial.println();
  }
}
