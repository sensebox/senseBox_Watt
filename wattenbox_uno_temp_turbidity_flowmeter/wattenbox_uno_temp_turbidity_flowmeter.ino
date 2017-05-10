#include <Arduino.h>

#include <Wire.h>
#include <RV8523.h>
#include <SPI.h>
#include <SdFat.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_LSM9DS1.h>

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5

float acceleration_x, acceleration_y, acceleration_z;

//Flowmeter
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

//Temp
#define ONE_WIRE_BUS 3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatur;


const uint8_t chipSelect = 4;
RV8523 rtc;
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

double voltage;
int truebung;



void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);


  rtc.start();
rtc.batterySwitchOver(1);

  SD.begin(chipSelect);
  //Temp
  sensors.begin();
  lsm.begin();
  setupSensor();
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);   


}

void loop() {
   

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
        conductiviy();
      break;
    case 4:
        battery();
       break;
    case 5:
        turbidity();
        break;
    case 6:
    accelerometer();
        break;
    case 7:
       char charFileName[nameOfFile.length()+1];
        nameOfFile.toCharArray(charFileName, sizeof(charFileName));
        file = SD.open(charFileName, FILE_WRITE);
        generateString();
        writeData(data);
        pulses = 0;
        break;
  }
  if (cases == 7){
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
  data += voltage;
  data += ";";
  data += truebung;
  data += ";";
  data += acceleration_x;
  data += ";";
  data += acceleration_y;
  data += ";";
  data += acceleration_z;
  data += ";";
  data += pulses;
  

}

void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}

void accelerometer(){
  lsm.read();  /* ask it to read in the data */ 

  /* Get a new sensor event */ 
  sensors_event_t a, m, g, temp;

  lsm.getEvent(&a, &m, &g, &temp); 

  acceleration_x = a.acceleration.x; 
  acceleration_y = a.acceleration.y;     
  acceleration_z = a.acceleration.z;
/*
  Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" gauss");
  Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" gauss");
  Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" gauss");

  Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" dps");
  Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" dps");
  Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" dps");
  */
}

void gettime(){
rtc.get(&sec, &min, &hour, &day, &month, &year);

}

void temp (){
 sensors.requestTemperatures();
  temperatur = sensors.getTempCByIndex(0);
}

void conductiviy(){
  
}

void battery(){
  voltage = analogRead(A1);
  voltage = ((voltage/1024)*5);
}

void turbidity(){
   truebung = analogRead(A2);
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
        file.println("date;time;temperatur;Battery_Voltage;Acceleration_x;Acceleration_y;Acceleration_z;"); //write the CSV Header to the file
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

