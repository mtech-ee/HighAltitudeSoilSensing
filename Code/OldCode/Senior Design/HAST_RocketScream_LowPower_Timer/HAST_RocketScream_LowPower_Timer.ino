/* Saturates transistor high periodically, turning on the Uno using the built in RTC.
Should also put Rocket Scream into ideal low power current consumption */

#include <RTCZero.h>
#include <LowPower.h>
/* For some reason the Rocketscream is missing the USBCore core library. This is not
necessary for the final prototype, but is included for initial demonstration purposes */
/* #include <USBCore.h> */

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 50;
const byte minutes = 58;
const byte hours = 10;

/* Change these values to set the current initial date */
const byte day = 6;
const byte month = 4;
const byte year = 20;

/* Create counters allowing us to set future wanted wakeup times */
int counterh = 1;
int counterm = 1;
int counters = 1;

void setup()
{
  #define Serial SerialUSB
  #define RTCZero

/* LED is for demonstration purposes to show visual wakeup.
Do not include in final prototype */
  pinMode(1, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);

  // initialize RTC 24H format
  rtc.begin(); 

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  // sets first interrupt time
  rtc.setAlarmTime(16, 0, 3); 
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  
  rtc.attachInterrupt(alarmMatch);

  /* Turn off all peripherals */
  ADC_OFF;
  BOD_OFF;
  SPI_OFF;
  USART0_OFF;
  USART1_OFF;
  TWI_OFF;
  TIMER0_OFF;
  TIMER1_OFF;
  TIMER2_OFF;
  TIMER3_OFF;
  TIMER4_OFF;
  
}
void sendDateTime(){
  
}

void loop()
{
  /* put the Rocketscream into standby mode */
  SerialUSB.println("Entering standby mode.");
  SerialUSB.println("RTC will wake the processor.");
  SerialUSB.println("Zzzz...");

  // Detach USB interface
  USBDevice.detach();
  // Enter standby mode
  LowPower.standby();  

  /* wakeup procedure once interrupt is triggered */
  // Attach USB interface
  USBDevice.attach();
  // Wait for serial USB port to open
  while(!SerialUSB);
  // Serial USB is blazing fast, you might miss the messages
  delay(3000);
  SerialUSB.println(" ");
  SerialUSB.println("Awake!");
  
  digitalWrite(LED_BUILTIN, HIGH);// just another visual to show it's awake

  /* Set pin 1 high to turn on the UNO.  The above wakeup procedure items do not need
  to be included in final prototype */
  digitalWrite(1, HIGH);
  delay(3000);

  /* determine new wakeup time */
  counterh = rtc.getHours();
  counterm = rtc.getMinutes()+1;
  counters = 0;
  /* make sure to reset minutes every hour or the alarm will be stuck looking for 
  a 60th minute */
  if (counterm >= 59) {
    counterm = 0;
    counterh = rtc.getHours()+1;
  }
  /* reset the hour to 1 every 24 hours */
  if (counterh >= 24) {
    counterh = 1;
  }
  /* set the alarm for the new wakeup time */
  rtc.setAlarmMinutes(counterm);
  delay(5000); //delay is for any final usb communication that is left
  digitalWrite(1, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void alarmMatch()
{
  /* leave this loop blank */
  
}
  
  
