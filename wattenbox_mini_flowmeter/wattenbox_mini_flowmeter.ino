#include <Arduino.h>

#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SdFat.h>

//flowmeter
#define FLOWSENSORPIN 2

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

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
int count = 1;
unsigned long start;
unsigned long now;
unsigned long elapsed;

float liters;

void generateString(){
  gettime();
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
  data += pulses;

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
        file.println("date;time;temperatur;lux"); //write the CSV Header to the file
        file.close();
        }
  }
  // serial output
  Serial.println(txt);
  // sd card output
  file.println(txt);
  file.flush();
}




void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
if (!rtc.begin()) {
  while (1);
}

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  SD.begin(chipSelect);
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);

}

void loop() {
  // put your main code here, to run repeatedly:
  float liters = pulses;
  Serial.println(count);
  Serial.println(start);
  Serial.println(now);
   Serial.print("Freq: "); Serial.println(flowrate);
  Serial.print("Pulses: "); Serial.println(pulses, DEC);
   if (count == 1){
     start = millis(); 
    count = 0;
    }
     now = millis();         // now: timestamp
     elapsed = now - start; // elapsed: duration
      if (elapsed >= 10000) {                   // comparing durations: OK
       char charFileName[nameOfFile.length()+1];
        nameOfFile.toCharArray(charFileName, sizeof(charFileName));
        file = SD.open(charFileName, FILE_WRITE);
        generateString();
        writeData(data);
        data = "";
        count = 1;
        pulses = 0;
        liters = 0;
       // start = 0;
        //now = 0;
        //elapsed = 0;
  }
}
  





