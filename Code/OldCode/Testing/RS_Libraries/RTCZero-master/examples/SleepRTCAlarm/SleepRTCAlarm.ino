/*
  Simple RTC Alarm for Arduino Zero

  Demonstrates the use an alarm to wake up an Arduino zero from Standby mode

  This example code is in the public domain

  http://arduino.cc/en/Tutorial/SleepRTCAlarm

  created by Arturo Guadalupi
  17 Nov 2015
*/

#include <RTCZero.h>

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 00;
const byte hours = 17;

/* Change these values to set the current initial date */
const byte day = 17;
const byte month = 11;
const byte year = 15;

void setup()
{
  Serial.begin(115200);

  rtc.begin();

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  rtc.setAlarmTime(17, 00, 10);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);

  rtc.attachInterrupt(alarmMatch);

  rtc.standbyMode();
}

void loop()
{

  Serial.println("Awake!");

  for (int i = 0; i < 10; i++)
    Serial.print(i);

  Serial.println();

  Serial.println("Done! Goodnight!");

  rtc.standbyMode();    // Sleep until next alarm match
}

void alarmMatch()
{

}
