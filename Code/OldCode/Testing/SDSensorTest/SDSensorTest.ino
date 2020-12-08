///////////////////////////////////////
////////////Sensor Block///////////////
///////////////////////////////////////
#include <SHT1x.h>

#define dataPin   8
#define clkPin    5

SHT1x sht1(dataPin,clkPin);
SHT1x sensors[] = {sht1};
int numSensors = sizeof(sensors) / sizeof(sensors[0]);

///////////////////////////////////////

///////////////////////////////////////
///////////////RTC Block///////////////
///////////////////////////////////////
#include <RTCZero.h>

RTCZero rtc;
// Change these values to set the initial time
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 18;
// Chanage these values to set the initial date
const byte day = 7;
const byte month = 2;
const byte year = 16;

///////////////////////////////////////

///////////////////////////////////////
/////////////SD/SPI Block//////////////
///////////////////////////////////////
#include <SPI.h>
#include <SD.h>

#define chipSelect  4

File dataFile;
File posFile;
///////////////////////////////////////

///////////////////////////////////////
//////////////General//////////////////
///////////////////////////////////////
String message;
const char msgEnd = ';';
const int daysPerMsg = 2;
String curMsg = "";


//////////////////////////////////////////////////////////////////////////////////////
// BEGIN MAIN FUNCTION
//////////////////////////////////////////////////////////////////////////////////////
void setup(){
  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);
  rtc.begin();
  rtc.setTime(hours,minutes,seconds);
  rtc.setDate(day,month,year);
  SerialUSB.begin(9600);
  while(!SerialUSB){
    ;// wait for USB connection to start-up
  }
  delay(5000);
  collectSensorData();
//readSD();
  

  
}


/////////////////////////////////////////////////////////////////////////////////////
/*BEGIN FUNCTION SECTION*/
//////////////////////////////////////////////////////////////////////////////////////
void collectSensorData(){
  message = DateTime() + ":";
  for(int i = 0; i < numSensors; i++){
    float temp = sensors[i].readTemperatureC();
    int humidity = (int) sensors[i].readHumidity();
    SerialUSB.println(temp);
    SerialUSB.println(humidity);
    message += " T:" + (String) temp + "/H:" + (String) humidity;
  }
  message += msgEnd;
}

String DateTime() {
  int dayNow = rtc.getDay();
  int monthNow = rtc.getMonth();
  int yearNow = rtc.getYear();

  int secondNow = rtc.getSeconds();
  int minuteNow = rtc.getMinutes();
  int hourNow = rtc.getHours();

  String stringDay = String(dayNow,DEC);
  String stringMonth = String(monthNow,DEC);
  String stringYear = String(yearNow,DEC);

  String stringSec = String(secondNow,DEC);
  String stringMin = String(minuteNow,DEC);
  String stringHour = String(hourNow,DEC);

  if (secondNow < 10){
    stringSec = makeItNice(stringSec);}
  if (minuteNow < 10){
    stringMin = makeItNice(stringMin);}
  if (hourNow < 10){
    stringHour = makeItNice(stringHour);}
  if (dayNow < 10){
    stringDay = makeItNice(stringDay);}
  if (monthNow < 10){
    stringMonth = makeItNice(stringMonth);}
  if (yearNow < 10) {
    stringYear = makeItNice(stringYear);}

  String dateNow = stringDay + "/" + stringMonth + "/" + stringYear + " " + stringHour + ":" + stringMin;
  return dateNow;
}
  
String makeItNice(String stringIn){
  //Inputs a 0 at the beginning of a string
  String  stringOut = "0" + stringIn;
  return stringOut;
}

/////////////////////////////////////////////////////////////////////////////////////
/*END FUNCTION SECTION*/
//////////////////////////////////////////////////////////////////////////////////////
// No need for loop yet, but for when actually implementing code, main will have to go here.
void loop(){
}
