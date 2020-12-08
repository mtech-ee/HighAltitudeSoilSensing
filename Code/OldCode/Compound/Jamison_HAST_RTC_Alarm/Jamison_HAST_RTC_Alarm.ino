/* Pulls pin high periodically, pulling relay pin high and turning on system*/

#include <RTCZero.h>
#include <LowPower.h>

/* Create an rtc object */
RTCZero rtc;
/* Change these values to set initial alarm time */
const byte aSeconds = 10;
const byte aMinutes = 0;
const byte aHours = 10;
/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 10;
/* Change these values to set the current initial date */
const byte day = 6;
const byte month = 4;
const byte year = 20;
/* Create counters allowing us to set future wanted wakeup times */
int counterh = 0;
int counterm = 1;
int counters = 10;


bool alarmFlg = false;
bool listenFlg = false;
bool sentFlg = false;

char handshake[18] = "What is the date?";
String dateNow;
char Amsg[18];
char Bmsg[15];

void setup()
{
  pinMode(3, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(3,LOW);
  digitalWrite(13, LOW);
  SerialUSB.begin(38400);
  Serial1.begin(38400);

  // initialize RTC 24H format
  rtc.begin(); 
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  // sets first interrupt time
  rtc.setAlarmTime(aHours, aMinutes, aSeconds); 
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(alarmMatch);

  /* Turn off all peripherals */
  /*ADC_OFF;
  BOD_OFF;
  SPI_OFF;
  USART0_OFF;
  USART1_OFF;
  TWI_OFF;
  TIMER0_OFF;
  TIMER1_OFF;
  TIMER2_OFF;
  TIMER3_OFF;
  TIMER4_OFF;*/
  
}

void loop()
{
  // Detach USB interface
  // USBDevice.detach();
  // Enter standby mode
  // LowPower.standby();  
  
  /* wakeup procedure once interrupt is triggered */
  // Attach USB interface
  // USBDevice.attach();
  
  // Wait for serial USB port to open
  //while(!SerialUSB);
  
  // Serial USB is blazing fast, you might miss the messages
  delay(3000);
  
  // SerialUSB.println(" ");
  // SerialUSB.println("Awake!");
  
  // Start communication protocols if the alarm has gone off
  if(alarmFlg == true){
    /* determine new wakeup time */
    UnoOn();
    recieveHandshake();
    sendMessage();
    recieveMessage();
    UnoOff();
    
    counterh = rtc.getHours();
    counterm = rtc.getMinutes()+5;
    counters = 0;

    /* make sure to reset minutes every hour or the alarm will be stuck looking for 
    a 60th minute */
  
    if (counterm >= 59) {
      counterm = 0;
      counterh = rtc.getHours()+1;
    }// end of minute reset
  
    /* reset the hour to 1 every 24 hours */
    if (counterh >= 24) {
    counterh = 1;
    }// end of hour reset
  
  /* set the alarm for the new wakeup time */
    rtc.setAlarmMinutes(counterm);

  }
  
  
}

void alarmMatch()
{
  alarmFlg = true;
  listenFlg = true;
}

String whatDate(){
  // retrieves the current time and date and outputs it as a 15 bit string
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

void UnoOn(){
  // Turns on Arduino and turns on LED for sight sake
  digitalWrite(3,HIGH);
  digitalWrite(13,HIGH);
  SerialUSB.println("Arduino turned on");
}

void UnoOff(){
  // Turns off Arduino and turns off LED
  digitalWrite(3,LOW);
  digitalWrite(13,LOW);
}

void recieveHandshake(){
  while(listenFlg == true){
    SerialUSB.println("Waiting for handshake");
    Serial1.readBytes(Amsg,18);
    SerialUSB.println(Amsg);  
      if(strcmp(Amsg,handshake) == 0){
        SerialUSB.println("There I go");
        SerialUSB.println("Handshake recieved");
        listenFlg = false;
      }// end if 2
    //}// end if 1
  }// end while
}// end recieveHanshake()

void sendMessage(){
  dateNow = whatDate();
  int str_len = dateNow.length() + 1;
  char toSend[str_len];
  dateNow.toCharArray(toSend,str_len);
  if(Serial1.available() == 0){
    Serial1.write(toSend,str_len);
    sentFlg = true;
  }// end if
}// end sendMessage()

void recieveMessage(){
  listenFlg = true;
  while(listenFlg == true){
    if(Serial1.available()>0){
      Serial1.readBytes(Bmsg,15);
      int str_lenB = dateNow.length() + 1;
      char toCmp[str_lenB];
      dateNow.toCharArray(toCmp,str_lenB);
      if(strcmp(Bmsg,toCmp) == 0){
        SerialUSB.println("date recieved");
        listenFlg = false;
      }// end if 2
    }// end if 1
  }// end while
}// end recieveMessage()
