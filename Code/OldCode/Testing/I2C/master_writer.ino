// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>
#include <RTCZero.h>

RTCZero rtc;

////////////////////////////////////////////////
/* Date/Time constant setup */
////////////////////////////////////////////////
// Change these values to set the initial time
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 1;
////////////////////////////////////////////////
// Chanage these values to set the initial date
const byte day = 7;
const byte month = 9;
const byte year = 20;
////////////////////////////////////////////////


void setup()
{
  pinMode(3,OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  digitalWrite(3,LOW);
  //digitalWrite(20,LOW);
  //digitalWrite(21,LOW);
  
  rtc.begin(); // initialize RTC 24H format
  rtc.setTime(hours,minutes,seconds);
  rtc.setDate(day,month,year);
  rtc.setAlarmTime(1,0,10);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);

  Wire.begin(); // join i2c bus (address optional for master)
  SerialUSB.begin(9600);
  rtc.attachInterrupt(alarmMatch);
  
}


void loop()
{

}

void alarmMatch(){
  digitalWrite(3,HIGH);
  digitalWrite(13,HIGH);
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
    stringSec = "0" + stringSec;
  }

  if (minuteNow < 10){
    stringMin = "0" + stringMin;
  }

  if (hourNow < 10){
    stringHour = "0" + stringHour;
  }

  if (dayNow < 10) {
    stringDay = "0" + stringDay;
  }

  if (monthNow < 10) {
    stringMonth = "0" + stringMonth;
  }

  if (yearNow < 10) {
    stringYear = "0" + stringYear;
  }

  
  String dateNow = stringDay + "/" + stringMonth + "/" + stringYear + " " + stringHour + ":" + stringMin + "?";
  
  int str_len = dateNow.length() + 1;
  char toSend[str_len];
  dateNow.toCharArray(toSend,str_len);
  delay(1000);
  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(toSend);        // sends date over i2c  
  Wire.endTransmission();    // stop transmitting
  delay(1000);
  SerialUSB.println(toSend);
}
