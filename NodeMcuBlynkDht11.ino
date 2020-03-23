
/*
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2019 mobizt
 *
 * This example is for FirebaseESP8266 Arduino library v 2.7.7 or later
*/

//This example shows the basic usage of Blynk platform and Firebase RTDB.

//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#include "DHTesp.h"
#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif


#define FIREBASE_HOST "Your Host Adres" //Without http:// or https:// schemes
#define FIREBASE_AUTH "Your database secret auth"
#define WIFI_SSID "Your wifi name"
#define WIFI_PASSWORD "your wifi password"

//Debug Blynk to serial port
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//Auth token for your Blynk app project
#define BLYNK_AUTH "Your Blynk Auth"

//Define FirebaseESP8266 data objects
FirebaseData humidityfire;
FirebaseData temperaturefire;


unsigned long sendDataPrevMillis = 0;


String mot = "/Motor/Int";
String tem = "/DHT11/Temperature";
String hum = "/DHT11/Humidity";
//D4 or GPIO2 on Wemos D1 mini
uint8_t BuiltIn_LED = 2;

uint16_t count = 0;



/*

Blynk app Widget setup 
**********************

1. Button Widget (Switch type), Output -> Virtual pin V1
2. LED Widget, Input -> Virtual pin V2
*/


WidgetLED led(V2);

DHTesp dht;

void setup()
{

  Serial.begin(115200);

  pinMode(BuiltIn_LED, OUTPUT);

  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);

  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
  // use this instead: 
  dht.setup(2, DHTesp::DHT11); // Connect DHT sensor t

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  humidityfire.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  humidityfire.setResponseSize(1024);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  temperaturefire.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
   temperaturefire.setResponseSize(1024);

 

  if (!Firebase.beginStream(humidityfire, hum))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + humidityfire.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }


  if (!Firebase.beginStream(temperaturefire, tem))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + temperaturefire.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASSWORD);
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Blynk.virtualWrite(V5, temperature);
  Blynk.virtualWrite(V6, humidity);

  Blynk.run();
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  delay(2000);
  
  Blynk.run();
   if (millis() - sendDataPrevMillis > 15000)
  {
    sendDataPrevMillis = millis();
    count++;

    Serial.println("------------------------------------");
    Serial.println("Set string...");
    if (Firebase.setString(humidityfire, hum , "NEM : %" + String(humidity)))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + humidityfire.dataPath());
      Serial.println("TYPE: " + humidityfire.dataType());
      Serial.print("VALUE: ");
    
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + humidityfire.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

if (Firebase.setString(temperaturefire, tem , "ISI :°C " + String(temperature)))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + temperaturefire.dataPath());
      Serial.println("TYPE: " + temperaturefire.dataType());
      Serial.print("VALUE: ");
    
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + temperaturefire.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

  }

  if (!Firebase.readStream(humidityfire))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't read stream data...");
    Serial.println("REASON: " + humidityfire.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  if (!Firebase.readStream(temperaturefire))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't read stream data...");
    Serial.println("REASON: " + temperaturefire.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (humidityfire.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
  if (temperaturefire.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
    
  if (humidityfire.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + humidityfire.streamPath());
    Serial.println("EVENT PATH: " + humidityfire.dataPath());
    Serial.println("DATA TYPE: " + humidityfire.dataType());
    Serial.println("EVENT TYPE: " + humidityfire.eventType());
    Serial.print("VALUE: "); 
    }
    Serial.println("------------------------------------");
    Serial.println();
  
  if (temperaturefire.streamAvailable())
  {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + temperaturefire.streamPath());
    Serial.println("EVENT PATH: " + temperaturefire.dataPath());
    Serial.println("DATA TYPE: " + temperaturefire.dataType());
    Serial.println("EVENT TYPE: " + temperaturefire.eventType());
    Serial.print("VALUE: "); 
    }
    Serial.println("------------------------------------");
    Serial.println();
  }



  
    
