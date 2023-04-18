
#define BLYNK_TEMPLATE_ID "xxxxx"
#define BLYNK_DEVICE_NAME "xxxxx"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD

#include "BlynkEdgent.h"
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include "GravityTDS.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define pinTDS 32
#define pinSuhu 25
#define pinpH 35
#define pinTurbidity 33
#define pinDO 34

#define tegRef 3.3
#define tegSensor 5
#define ADCRef 4095.0

#define tegCalDO (1860)
#define suhuCalDO (27.2)

const char* serverName = "https://www.server.com/post-water-data.php";
String apiKeyValue = "ju&o9*bg";

const uint16_t tabelDO[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};
 
uint8_t suhuDO;
uint16_t analogDO;
uint16_t teganganDO;
uint16_t DO;
 
int16_t nilaiDO(uint32_t tegTabelDO, uint8_t suhuTabelDO)
{
  uint16_t tegJenuh = (uint32_t)tegCalDO + (uint32_t)35 * suhuTabelDO - (uint32_t)suhuCalDO * 35;
  return ((tegTabelDO * tabelDO[suhuTabelDO]) / (tegJenuh));
}

OneWire oneWire(pinSuhu);
DallasTemperature suhu(&oneWire);
GravityTDS TDS;

float nilaiTDS = 0;
float nilaiEC = 0;
float EC =0;
float nilaiRho = 0;
float nilaiSalt = 0;
float teganganTurbidity;
float nilaiTurbidity;

void setup() {
  Serial.begin(115200);
  suhu.begin();
  EEPROM.begin(72);
  TDS.setPin(pinTDS);
  TDS.setAref(tegRef);
  TDS.setAdcRange(ADCRef);
  TDS.begin();
}

  void uploadToBlynk() {
    Blynk.virtualWrite(V0,  sensorSuhu());
    Blynk.virtualWrite(V1,  sensorpH());
    Blynk.virtualWrite(V2,  sensorTDS());    
    Blynk.virtualWrite(V3,  sensorEC());
    Blynk.virtualWrite(V4,  sensorRho());
    Blynk.virtualWrite(V5,  sensorSalt);
    Blynk.virtualWrite(V6,  sensorTurbidity());
    Blynk.virtualWrite(V6,  sensorDO());
  }

void loop() {
  Serial.println(sensorSuhu(),3);
  Serial.println(sensorpH(),3);
  Serial.println(sensorTDS(),3);
  Serial.println(sensorEC(),3);
  Serial.println(sensorRho(),3);
  Serial.println(sensorSalt(),6);
  Serial.println(sensorTurbidity(),3);
  Serial.println(sensorDO(),3);
  Serial.println("============");
  kirimData();
  delay(1000);
}

float sensorSuhu(){
  suhu.requestTemperatures(); 
  float nilaiSuhuCelcius = suhu.getTempCByIndex(0); 
  return nilaiSuhuCelcius;
  }

float sensorpH(){
  int nilaiAnalogpH = analogRead(pinpH) + 321;
  float teganganpH = (nilaiAnalogpH) * (tegRef /  ADCRef);
  float nilaipH = (teganganpH) * (14/tegRef);
  return nilaipH;
}

float sensorTDS(){
  suhu.requestTemperatures();
  TDS.setTemperature(suhu.getTempCByIndex(0));
  TDS.update();
  nilaiTDS = TDS.getTdsValue();
  return nilaiTDS;
  }

float sensorEC(){
  suhu.requestTemperatures();
  TDS.setTemperature(suhu.getTempCByIndex(0));
  TDS.update();
  nilaiEC = TDS.getEcValue();
  return nilaiEC;
  }

float sensorRho(){
  suhu.requestTemperatures();
  TDS.setTemperature(suhu.getTempCByIndex(0));
  TDS.update();
  EC = TDS.getEcValue();
  nilaiRho = 1000 / EC;
  return nilaiRho;
  }

float sensorSalt(){
  suhu.requestTemperatures();
  TDS.setTemperature(suhu.getTempCByIndex(0));
  TDS.update();
  nilaiSalt = TDS.getEcValue()*0.000446;
  return nilaiSalt;
  }

float sensorTurbidity(){
  teganganTurbidity = 0;
  for(int i=0; i<800; i++)
  {teganganTurbidity += ((float)analogRead(pinTurbidity)/ADCRef)*tegSensor;}
  teganganTurbidity = teganganTurbidity/800;
  if(teganganTurbidity < 2.5){nilaiTurbidity = 3000;}
  else if (teganganTurbidity > 4.2){nilaiTurbidity = 0;}
  else{nilaiTurbidity = (-1120.40*teganganTurbidity*teganganTurbidity)+(5742.30*teganganTurbidity)-(4452.80); }
  return nilaiTurbidity;
  }

float sensorDO(){
  suhu.requestTemperatures();
  suhuDO = suhu.getTempCByIndex(0);
  analogDO = analogRead(pinDO);
  teganganDO = uint32_t(tegRef/1000) * analogDO / ADCRef;
  return nilaiDO(teganganDO, suhuDO)/1000;
  } 

void kirimData(){
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = "api_key=" + apiKeyValue + "&Suhu=" + String(sensorSuhu()) 
                          + "&pH=" + String(sensorpH()) + "&TDS=" + String(sensorTDS())
                          + "&EC=" + String(sensorEC()) + "&Kekeruhan=" + String(sensorTurbidity())
                          + "&DO=" + String(sensorDO())+ "&Resistivity=" + String(sensorRho())
                          + "&Salt=" + String(sensorSalt())+"";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    int httpResponseCode = http.POST(httpRequestData);
   if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
   } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
   }
    http.end();
   } else {
    Serial.println("WiFi Disconnected");
   }
  }