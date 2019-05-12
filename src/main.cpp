#include <Arduino.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>
#include <Scheduler.h>
// #include "./DHTTask.cpp"
#define FIREBASE_HOST "smartfarm-50309.firebaseio.com"
#define FIREBASE_AUTH "E8Mq53C112QdluHI32ANRaZyVXsIlLFvsIpDVat4"
#define DHTTYPE DHT11

#define DHTPIN D3
// #define WIFI_SSID "99/246"
// #define WIFI_PASSWORD "homesweethome"
#define WIFI_SSID "Nextzy"
#define WIFI_PASSWORD "eyeshield"
unsigned long sendDataPrevMillis = 0;

String path = "/users/qweasdzxc";

uint16_t count = 0;
FirebaseData firebaseData;
DHT dht(DHTPIN, DHTTYPE);

class DHTTask : public Task
{
  void DHTRunning()
  {
    delay(2000);

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));
  }

protected:
  void setup()
  {
    dht.begin();
  }
  void loop()
  {
    DHTRunning();
  }

private:
  uint8_t state;
} dht_task;

class FirebaseTask : public Task
{
  void setupFirebase()
  {
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
  void streamFirebase()
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

protected:
  void setup()
  {
    setupFirebase();
  }
  void loop()
  {
    streamFirebase();
  }

private:
  uint8_t state;
} firebase_task;

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
}
void setup()
{
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(D1, OUTPUT);
  setupWifi();
  // setupFirebase();

  Scheduler.start(&dht_task);
  Scheduler.start(&firebase_task);
  Scheduler.begin();
}
void loop()
{
}
