#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <MqttConnector.h>
#include <Ticker.h>
#include "CMMC_Interval.hpp"
#include "init_mqtt.h"
#include <Wire.h>
#include "SSD1306.h"

const int sensorIn = A0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
int mVperAmp = 100; // use 100 for 20A Module and 66 for 30A Module

#include "_publish.h"
#include "_receive.h"

Ticker ticker;
CMMC_Interval timer001;

/* WIFI INFO */
#ifndef WIFI_SSID
#define WIFI_SSID        "DEVICES-AP"
#define WIFI_PASSWORD    "devicenetwork"
#endif

String MQTT_HOST        = "mqtt.cmmc.io";
String MQTT_USERNAME    = "";
String MQTT_PASSWORD    = "";
String MQTT_CLIENT_ID   = "";
String MQTT_PREFIX      = "CMMC";
int    MQTT_PORT        = 1883;

int PUBLISH_EVERY       = 3000;

MqttConnector *mqtt;

#define WiFi_Logo_width 128 // 60
#define WiFi_Logo_height 64 // 36
const char WiFi_Logo_bits[] PROGMEM = {
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0x3F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0x3F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0x03, 0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x80, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00,
  0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0x1F, 0xE0, 0xFF, 0xFF, 0x1F, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xE0, 0xFF, 0xFF, 0x1F, 0xF8, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xFE, 0xFF,
  0xFF, 0x7F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0x07, 0xFE, 0xFF, 0xFF, 0x7F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x81, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF,
  0xFF, 0xFF, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xFC,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1, 0xFF, 0xFF,
  0xFF, 0xFF, 0x07, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0x3F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0x3F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFC,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0x3F, 0xE0, 0xFF, 0xFF,
  0xFF, 0xFF, 0x1F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0x3F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFE, 0xFF, 0xFF, 0x7F, 0x60, 0xFC,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xE0, 0xFF,
  0xFF, 0x07, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0x01, 0xE0, 0xFF, 0xFF, 0x07, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0x0F, 0xE0, 0x01, 0xFC, 0xFF, 0x81, 0x1F, 0xF0,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0x0F, 0xE0, 0x01, 0xFC,
  0xFF, 0x81, 0x1F, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xC0, 0xE1, 0x1F, 0xF0, 0x0F, 0xE0, 0x87, 0x03, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xC0, 0xE1, 0x1F, 0xF0, 0x0F, 0xE0, 0x87, 0x03,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x0F, 0xF0, 0xE1, 0xFF, 0x03,
  0x00, 0xFE, 0x87, 0x0F, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x0F,
  0xF0, 0xE1, 0xFF, 0x03, 0x00, 0xFE, 0x87, 0x0F, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0x0F, 0xFF, 0x87, 0xFF, 0x0F, 0xF0, 0xFF, 0x87, 0xFF,
  0xF0, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x0F, 0xFF, 0x87, 0xFF, 0xFF,
  0xFC, 0xFF, 0xE1, 0x3F, 0xF0, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x0F,
  0xFF, 0x87, 0xFF, 0xFF, 0xFC, 0xFF, 0xE1, 0x3F, 0xF0, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0x0F, 0xFF, 0x1F, 0xFE, 0xFF, 0xFF, 0x7F, 0xE0, 0x3F,
  0xF0, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x0F, 0xFF, 0x1F, 0xFE, 0xFF,
  0xFF, 0x7F, 0xE0, 0x3F, 0xF0, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x3F,
  0xFF, 0x1F, 0xF8, 0xFF, 0xFF, 0x7F, 0xF8, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0x3F, 0xFF, 0x1F, 0xF8, 0xFF, 0xFF, 0x7F, 0xF8, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0xE0, 0xFF,
  0xFF, 0x1F, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0x00, 0x00, 0x80, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0x07, 0x00, 0x00,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xFC,
  0x3F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x07, 0xFC, 0x3F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0xF8, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x00,
  0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xFC,
  0x3F, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0x81, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00,
  0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x07, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x80, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00,
  0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x1F, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF,
};

SSD1306  display(0x3c, 4, 5);

float getVPP()
{
  float result;

  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the maximum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 5) / 1024.0;

  return result;
}

void init_hardware()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Starting...");
  pinMode(LED_BUILTIN, OUTPUT);
}

void init_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf ("Connecting to %s:%s\r\n", WIFI_SSID, WIFI_PASSWORD);
    delay(300);
  }
  Serial.println("WiFi Connected.");
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup()
{
  init_hardware();
  init_wifi();
  init_mqtt();

  display.init();
  display.flipScreenVertically();
  display.drawXbm(0, 0, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.display();
  delay(2000);
  display.clear();
}

void loop()
{
  display.clear();
  Voltage = getVPP();
  VRMS = (Voltage / 2.0) * 0.707;
  AmpsRMS = (VRMS * 100) / mVperAmp;

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 15, "CMMC plug003");
  display.setFont(ArialMT_Plain_10);
  display.drawString(15, 40, "Current = ");
  display.drawString(65, 40, String(AmpsRMS));
  display.drawString(85, 40, " Amps");
  display.drawString(30, 50, "RELAY =");
  display.drawString(80, 50, String(pin_state));
  display.display();

  Serial.print(AmpsRMS);
  Serial.println(" Amps RMS");

  mqtt->loop();
}
