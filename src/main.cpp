#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "secrets.h"

#ifndef SERIAL_DEBUG
#define SERIAL_DEBUG 1
#endif

#if SERIAL_DEBUG
#define DBG_PRINT(x) Serial.print(x)
#define DBG_PRINTLN(x) Serial.println(x)
#else
#define DBG_PRINT(x) ((void)0)
#define DBG_PRINTLN(x) ((void)0)
#endif

// DHT setup
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

void connectWiFi() {
  DBG_PRINT("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    yield();
    DBG_PRINT('.');
    if (millis() - start > 20000) {
      DBG_PRINTLN("\nWiFi connect timeout, retrying...");
      start = millis();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
  }
  DBG_PRINTLN();
  DBG_PRINT("Connected, IP: ");
  DBG_PRINTLN(WiFi.localIP());
}

bool postToSupabase(float temperature) {
  BearSSL::WiFiClientSecure client;
  client.setInsecure(); // skip cert validation (recommended: pin CA in production)

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/" + SUPABASE_TABLE;
  if (!http.begin(client, url)) {
    DBG_PRINTLN("HTTP begin failed");
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
  http.addHeader("Prefer", "return=representation");

  StaticJsonDocument<128> doc;
  doc["temperature"] = temperature;

  String payload;
  serializeJson(doc, payload);

  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    DBG_PRINT("HTTP response code: ");
    DBG_PRINTLN(httpCode);
    DBG_PRINTLN(http.getString());
  } else {
    DBG_PRINT("POST failed, error: ");
    DBG_PRINTLN(http.errorToString(httpCode));
  }

  http.end();
  return (httpCode >= 200 && httpCode < 300);
}

void setup() {
#if SERIAL_DEBUG
  Serial.begin(115200);
  delay(100);
#endif

  pinMode(DHT_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);

  dht.begin();
  delay(2000);
  connectWiFi();
}

void blinkUploadIndicator() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  static unsigned long lastUploadTime = 0;
  static unsigned long lastSampleTime = 0;
  static float sumTemperature = 0.0;
  static int sampleCount = 0;
  const unsigned long UPLOAD_INTERVAL = 30000;
  const unsigned long SAMPLE_INTERVAL = 2000;
  unsigned long currentMillis = millis();

  if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = currentMillis;
    yield();

    float sample = NAN;
    for (int attempt = 0; attempt < 4; attempt++) {
      sample = dht.readTemperature();
      if (!isnan(sample)) {
        break;
      }
      delay(1000);
      yield();
    }

    if (!isnan(sample)) {
      sumTemperature += sample;
      sampleCount++;
      DBG_PRINT("Sampled temperature: ");
      DBG_PRINT(sample);
      DBG_PRINTLN(" °C");
    }
  }

  if (currentMillis - lastUploadTime < UPLOAD_INTERVAL) {
    return;
  }

  lastUploadTime = currentMillis;

  // heartbeat blink to confirm operation
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  yield();

  if (sampleCount == 0) {
    DBG_PRINTLN("No valid temperature samples collected");
    blinkUploadIndicator();
    return;
  }

  float temp = sumTemperature / sampleCount;
  sumTemperature = 0.0;
  sampleCount = 0;

  DBG_PRINT("Mean temperature: ");
  DBG_PRINT(temp);
  DBG_PRINTLN(" °C");

  bool ok = postToSupabase(temp);
  if (ok) {
    DBG_PRINTLN("Logged to Supabase");
  } else {
    DBG_PRINTLN("Upload failed");
  }

  blinkUploadIndicator();
}
