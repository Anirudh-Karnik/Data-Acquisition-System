#include "DHT.h"
#include <Wire.h>;
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define DHTPIN 2  
#define DHTTYPE DHT11
#define ardI2CAddress 0x39

#define LED1_PIN 6
#define LED2_PIN 7
#define LED3_PIN 8
#define LED4_PIN 9
#define LED5_PIN 10
#define LED6_PIN 11
#define LED7_PIN 12
#define LED8_PIN 13
#define BUZ_PIN 5

float t=0, latitude=0, longitude=0, dateDay=0, dateMonth=0, dateYear=0, timeHour=0, timeMinute=0;
byte tBytes[4], hBytes[4], dataBytes[19], latBytes[4], lngBytes[4], dateDayBytes[1], dateMonthBytes[1], dateYearBytes[2], timeHourBytes[1], timeMinuteBytes[1];
int h=0, prevSentencesWithFix=0, currSentencesWithFix=0;
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
String sentFromNMCU="";

DHT dht(DHTPIN, DHTTYPE);
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

union FLT_TO_BYTES{
  float fltToConv;
  byte rsltntFltBytes[4];
};

union INT_TO_BYTES{
  int intToConv;
  byte rsltntIntBytes[1];
};

union FLT_TO_BYTES floatToBytes;
union INT_TO_BYTES intToBytes;

void requestEvent()
{
  Wire.write(dataBytes, 19); 
}


void receiveEvent()
{
//  if(Wire.read()==2){
//    sentFromNMCU="2";
//  }
sentFromNMCU=Wire.read();
}


void getDHTData()
{
  t = dht.readTemperature();      //passing true as an argument means Farenheit, else Celsius
  h = dht.readHumidity();

  if (isnan(t)||isnan(h)) 
  {
    return;
  }

  floatToBytes.fltToConv=t;
  tBytes[0] = floatToBytes.rsltntFltBytes[0];
  tBytes[1] = floatToBytes.rsltntFltBytes[1];
  tBytes[2] = floatToBytes.rsltntFltBytes[2];
  tBytes[3] = floatToBytes.rsltntFltBytes[3];

  intToBytes.intToConv=h;
  hBytes[0] = intToBytes.rsltntIntBytes[0];

}

void getGPSData()
{
  if (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }

      if (gps.date.isValid())
      {
        dateDay=gps.date.day();
        dateMonth=gps.date.month();
        dateYear=gps.date.year();
      }

      if(gps.time.isValid())
      {
        timeHour=gps.time.hour();
        timeMinute=gps.time.minute();
      }
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

  if(gps.sentencesWithFix()>prevSentencesWithFix){
    digitalWrite(LED3_PIN, LOW);
    digitalWrite(BUZ_PIN, HIGH);
    delay(10);
    digitalWrite(LED3_PIN, HIGH);
    digitalWrite(BUZ_PIN, LOW);
    prevSentencesWithFix=gps.sentencesWithFix();
  }
  
  floatToBytes.fltToConv=latitude;
  latBytes[0]=floatToBytes.rsltntFltBytes[0];
  latBytes[1]=floatToBytes.rsltntFltBytes[1];
  latBytes[2]=floatToBytes.rsltntFltBytes[2];
  latBytes[3]=floatToBytes.rsltntFltBytes[3];

  floatToBytes.fltToConv=longitude;
  lngBytes[0]=floatToBytes.rsltntFltBytes[0];
  lngBytes[1]=floatToBytes.rsltntFltBytes[1];
  lngBytes[2]=floatToBytes.rsltntFltBytes[2];
  lngBytes[3]=floatToBytes.rsltntFltBytes[3];

  intToBytes.intToConv=dateDay;
  dateDayBytes[0] = intToBytes.rsltntIntBytes[0];

  intToBytes.intToConv=dateMonth;
  dateMonthBytes[0] = intToBytes.rsltntIntBytes[0];

  intToBytes.intToConv=dateYear;
  dateYearBytes[0] = intToBytes.rsltntIntBytes[0];
  dateYearBytes[1] = intToBytes.rsltntIntBytes[1];

  intToBytes.intToConv=timeHour;
  timeHourBytes[0] = intToBytes.rsltntIntBytes[0];

  intToBytes.intToConv=timeMinute;
  timeMinuteBytes[0] = intToBytes.rsltntIntBytes[0];
}


void getSensorData()
{
  getDHTData();
  getGPSData();

  //putting DHT and GPS data in the Bytes array
  dataBytes[0]=tBytes[0];
  dataBytes[1]=tBytes[1];
  dataBytes[2]=tBytes[2];
  dataBytes[3]=tBytes[3];
  dataBytes[4]=hBytes[0];
  dataBytes[5]=latBytes[0];
  dataBytes[6]=latBytes[1];
  dataBytes[7]=latBytes[2];
  dataBytes[8]=latBytes[3];
  dataBytes[9]=lngBytes[0];
  dataBytes[10]=lngBytes[1];
  dataBytes[11]=lngBytes[2];
  dataBytes[12]=lngBytes[3];
  dataBytes[13]=dateDayBytes[0];
  dataBytes[14]=dateMonthBytes[0];
  dataBytes[15]=dateYearBytes[0];
  dataBytes[16]=dateYearBytes[1];
  dataBytes[17]=timeHourBytes[0];
  dataBytes[18]=timeMinuteBytes[0];
}



void setup() 
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Wire.begin(ardI2CAddress);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  dht.begin();
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);
  pinMode(LED6_PIN, OUTPUT);
  pinMode(LED7_PIN, OUTPUT);
  pinMode(LED8_PIN, OUTPUT);
  pinMode(BUZ_PIN, OUTPUT);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  digitalWrite(LED3_PIN, HIGH);
  digitalWrite(LED4_PIN, HIGH);
  digitalWrite(LED5_PIN, HIGH);
  digitalWrite(LED6_PIN, HIGH);
  digitalWrite(LED7_PIN, HIGH);
  digitalWrite(LED8_PIN, HIGH);
  digitalWrite(BUZ_PIN, LOW);
}


void loop() 
{

  
  if(sentFromNMCU=="2"){
    digitalWrite(LED1_PIN, LOW);
  }
  if(sentFromNMCU=="3"){
    digitalWrite(LED4_PIN, LOW);
    sentFromNMCU="";
    delay(10);
    digitalWrite(LED4_PIN, HIGH);
  }
  else{
    digitalWrite(LED4_PIN, HIGH);
  }
  if(sentFromNMCU=="4"){
    digitalWrite(LED5_PIN, LOW);
    delay(10);
    digitalWrite(LED5_PIN, HIGH);
  }
  else{
    digitalWrite(LED5_PIN, HIGH);
  }
  
    getSensorData();
}
