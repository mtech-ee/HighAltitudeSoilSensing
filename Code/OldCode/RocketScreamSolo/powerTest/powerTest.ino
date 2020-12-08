/**** INCLUDES ****/
#include <LowPower.h>
#include <RTCZero.h>
#include <SHT1x.h>
// #include <SPI.h> will add SD functionality at a later point
// #include <SD.h>
#include <IridiumSBD.h>
/*****************/

/**** SENSORS ****/
/*NOTE: more can be added at a later point*/
#define dataPin1 9
#define clockPin 5

SHT1x sht1(dataPin1, clockPin);
SHT1x sensors [] = {sht1};
int numSensors = sizeof(sensors) / sizeof(sensors[0]);
/*****************/

/**** ROCKBLOCK ****/
#define IridiumSerial Serial
IridiumSBD modem(IridiumSerial);
const char msgEnd = ';';
/*******************/

/**** REAL TIME CLOCK ****/
RTCZero rtc;
/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 1;

/* Change these values to set the current initial date */
const byte day = 7;
const byte month = 7;
const byte year = 20;
/************************/

/**** GLOBAL VARIABLES ****/
bool alarmFlg = false;
bool usbFlag = true;
bool initFlag = true;
bool opFlag = true;
int watcher = 0;
String measurement;

/**************************/
void setup()

{
    delay(1500);
    SerialUSB.begin(9600);
    delay(5000);
    rtc.begin();
    rtc.setTime(hours,minutes,seconds);
    rtc.setDate(day,month,year);
    rtc.setAlarmTime(1,0,30);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(alarmMatch);
    //ADC_OFF;
    //BOD_OFF;
    //SPI_OFF;
    //USART0_OFF;
    //USART1_OFF;
    //TWI_OFF;
    //TIMER0_OFF;
    //TIMER1_OFF;
    //TIMER2_OFF;
    //TIMER3_OFF;
    //TIMER4_OFF;
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
    //pinMode(4,OUTPUT);
    //digitalWrite(4,HIGH);
    

    

}

void loop() 
{   
  if(alarmFlg == true){
    USBDevice.attach();
    delay(500);
    SerialUSB.begin(9600);
    SerialUSB.println("Hello, I'm awake!");
    watcher = millis();
    if(opFlag == true){
      collectDate();
      soilSensor();
      // sdCollect();
      // sendMessage();
      opFlag = false;
    }

    //USART1_ON;
    //ADC_ON;
    //TWI_ON;
    //TIMER0_ON;

    SerialUSB.println(watcher);
    if(watcher == 60000){
      
      SerialUSB.println("Printing Measurements: ");
      while(1){
        SerialUSB.println(measurement);
        delay(500);
      }
    }
  }
  if((alarmFlg == false) && (initFlag == true)){
    SerialUSB.println("Going into Sleep Mode");
    if(usbFlag == true){
      USBDevice.detach();
      usbFlag = false;
    }
    LowPower.standby();
    initFlag = false;
  }

}
void alarmMatch(){
  alarmFlg = true;
}

void soilSensor(){
  for(int i = 0; i<numSensors;i++){
    float temp = sensors[i].readTemperatureC();
    int humidity = (int) sensors[i].readHumidity();
    measurement += " T:" + (String) temp + "/H:" + (String) humidity + ",";
  }
  measurement += msgEnd;
}

void collectDate(){
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
    
  if (yearNow < 10){
    stringYear = makeItNice(stringYear);}

  String dateNow = stringDay + "/" + stringMonth + "/" + stringYear + " " + stringHour + ":" + stringMin;
  measurement += dateNow;
}

String makeItNice(String stringIn){
  //Inputs a 0 at the beginning of a string
  String  stringOut = "0" + stringIn;
  return stringOut;
}
