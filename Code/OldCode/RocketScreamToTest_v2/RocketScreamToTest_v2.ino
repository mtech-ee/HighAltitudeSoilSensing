#include <RTCZero.h>
//#include <ArduinoLowPower.h>
#include <IridiumSBD.h>
#include <SD.h>
//#include <LowPower.h>
RTCZero rtc;


////////////////////////////////////////////////
/* Date/Time constant setup */
////////////////////////////////////////////////
// Change these values to set the initial time
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 7;
////////////////////////////////////////////////
// Chanage these values to set the initial date
const byte day = 9;
const byte month = 1;
const byte year = 63;
////////////////////////////////////////////////
// Change these values to set the initial alarm time
const byte Aseconds = 10;
const byte Aminutes = 0;
const byte Ahours = 7;
////////////////////////////////////////////////

bool alarmFlg = false;
bool listenFlg = false;
bool sentFlg = false;
bool doneFlg = false;

char handshake[18] = "What is the date?";
String dateNow;
char Amsg[18];
char Bmsg[15];


void setup() {
  pinMode(3,OUTPUT);
  digitalWrite(3,LOW);


  rtc.begin(); // initialize RTC 24H format
  rtc.setTime(hours,minutes,seconds);
  rtc.setDate(day,month,year);
  rtc.setAlarmTime(Ahours,Aminutes,Aseconds);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);

  Serial1.begin(9600);
  SerialUSB.begin(9600);
  rtc.attachInterrupt(alarmMatch);

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

void loop() {
  if(alarmFlg == true){
      UnoOn();
      recieveHandshake();
      sendMessage();
      recieveMessage();
      UnoOff();
      alarmFlg = false;
  }
}

void recieveHandshake(){
  while(listenFlg == true){
    SerialUSB.println("Waiting for handshake");
    SerialUSB.println(Amsg);
    delay(1000);
    SerialUSB.println("Recieving");
    Serial1.readBytes(Amsg,18);
    SerialUSB.println(Amsg);
    if(strcmp(Amsg,handshake) == 0){
      listenFlg = false;
    }// end if 1
  }// end while
}// end recieveMessage()

void alarmMatch(){
  alarmFlg = true;
  listenFlg = true;
}

void sendMessage(){
  dateNow = whatDate();
  int str_len = dateNow.length() + 1;
  char toSend[str_len];
  dateNow.toCharArray(toSend,str_len);
  if(Serial1.available() ==0){
    Serial1.write(toSend,str_len);
    SerialUSB.println(toSend);
    sentFlg = true;
  }// end if
  
  
}

void recieveMessage(){
  listenFlg = true;
  while(listenFlg == true){
    Serial1.readBytes(Bmsg,15);
    int str_lenB = dateNow.length() + 1;
    char toCmp[str_lenB];
     dateNow.toCharArray(toCmp,str_lenB);
     if(strcmp(Bmsg,toCmp) == 0){
       SerialUSB.println("Collection complete, turning off");
       listenFlg = false;
       doneFlg = true;
    }
  }
}
String whatDate(){
  
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

void flgReset(){
  alarmFlg = false;
  listenFlg = true;
  sentFlg = false;
}
void UnoOn(){
  digitalWrite(3,HIGH);
  digitalWrite(13,HIGH);
  
}
void UnoOff(){
  digitalWrite(3,LOW);
  digitalWrite(13,LOW);
}
