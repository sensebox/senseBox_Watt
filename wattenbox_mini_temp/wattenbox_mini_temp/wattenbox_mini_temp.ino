#include <Arduino.h>

#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SdFat.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Makerblog_TSL45315.h>

//Lichtsensor
Makerblog_TSL45315 luxsensor = Makerblog_TSL45315(TSL45315_TIME_M4)
uint32_t lux;
//Temp
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatur;


const uint8_t chipSelect = 10;
RTC_PCF8523 rtc;
SdFat SD;

//SD-File
File file;
String nameOfFile;

boolean firstTime = true;

//RTC
uint8_t sec , min, hour, day, month;
uint16_t year;
unsigned long starttime;
unsigned long sampletime_ms = 30000;

String data = "";
int cases = 1;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
if (!rtc.begin()) {
  while (1);
}

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  SD.begin(chipSelect);
  //Temp
  sensors.begin();
  luxsensor.begin()

}

void loop() {
  // put your main code here, to run repeatedly:
  switch (cases){
    case 1:
       sleep(30000);
       data = "";
       gettime();
       break;
    case 2:
        temp();
        break;
    case 3:
      lux = luxsensor.readLux();
      break;
     case 4:
       char charFileName[nameOfFile.length()+1];
        nameOfFile.toCharArray(charFileName, sizeof(charFileName));
        file = SD.open(charFileName, FILE_WRITE);
        generateString();
        writeData(data);

  }
  if (cases == 4){
    cases = 1;
      }
  else cases++;
}

void generateString(){
  data += day;
  data += ".";
  data += month;
  data += ".";
  data += year;
  data += ";";
  data += hour;
  data += ":";
  data += min;
  data += ":";
  data += sec;
  data += ";";
  data += temperatur;
  data += ";";
  data += lux;

}

void gettime(){
  DateTime now = rtc.now();
  year = now.year();
  month = now.month();
  day = now.day();
  hour = now.hour();
  min = now.minute();
  sec = now.second();

}

void temp (){
 sensors.requestTemperatures();
  temperatur = sensors.getTempCByIndex(0);
  data += temperatur;
  data += ";";

}

String generateFileName(String boardID)
{
  String fileName = String();
  unsigned int filenumber = 1;
  boolean isFilenameExisting;
  do{
    fileName = boardID;
    fileName += "-";
    fileName += filenumber;
    fileName += ".csv";
    Serial.println(fileName);
    char charFileName[fileName.length() + 1];
    fileName.toCharArray(charFileName, sizeof(charFileName));

    filenumber++;

    isFilenameExisting = SD.exists(charFileName);
  }
  while(isFilenameExisting);

   Serial.print("Generated filename: ");
   Serial.println(fileName);

   return fileName;
}

// write the data to the output channels
void writeData(String txt) {
  //// if first time a text should be outputted, output the column names first
  if (firstTime) {

      nameOfFile = generateFileName("a01");
      char charFileName[nameOfFile.length()+1];
      nameOfFile.toCharArray(charFileName, sizeof(charFileName));
      firstTime = false;
      file = SD.open(charFileName, FILE_WRITE);
      if (file) {
        file.println("date;time"); //write the CSV Header to the file
        file.close();
        }
  }
  // serial output
  Serial.println(txt);
  // sd card output
  file.println(txt);
  file.flush();
}

void sleep(unsigned long ms) {            // ms: duration
  unsigned long start = millis();         // start: timestamp
  for (;;) {
    unsigned long now = millis();         // now: timestamp
    unsigned long elapsed = now - start;  // elapsed: duration
    if (elapsed >= ms)                    // comparing durations: OK
      return;
  }
}
