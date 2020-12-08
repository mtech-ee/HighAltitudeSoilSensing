#include <SHT1x.h>
#include <IridiumSBD.h>
#include <RTCZero.h>
RTCZero rtc;
#define IridiumSerial Serial1
/***************************/
//  Global Variable init   //
String message;
const char msgEnd = ';';
#define dataPin 9
#define clockPin 5
SHT1x sht1(dataPin,clockPin);
SHT1x sensors[] = {sht1};
int numSensors = sizeof(sensors) / sizeof(sensors[0]);
IridiumSBD modem(IridiumSerial);

//         RTC init        //
// Change these values to set the initial time
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 11;
// Chanage these values to set the initial date
const byte day = 6;
const byte month = 23;
const byte year = 20;
/***************************/

void setup() {
  
  int signalQuality = -1;
  int err;
  
  rtc.begin(); // initialize RTC 24H format
  rtc.setTime(hours,minutes,seconds);
  rtc.setDate(day,month,year);
  
  // Start PC serial port
  SerialUSB.begin(115200);
  while(!SerialUSB);
  // Start modem serial port
  IridiumSerial.begin(19200);
  // Begin satellite modem operations
  SerialUSB.println("Starting modem...");
  err = modem.begin();
  if (err != ISBD_SUCCESS){
    SerialUSB.println("Begin failed: error ");
    SerialUSB.println(err);
    if (err == ISBD_NO_MODEM_DETECTED){
      SerialUSB.println("No modem detected: check wiring.");
      return;
    }
  }
  delay(10000);
  collectSensorData();
  // Example: Print the firmware revision
  char version[12];
  err = modem.getFirmwareVersion(version, sizeof(version));
  if (err != ISBD_SUCCESS)
  {
     SerialUSB.print("FirmwareVersion failed: error ");
     SerialUSB.println(err);
     return;
  }
  SerialUSB.print("Firmware Version is ");
  SerialUSB.print(version);
  SerialUSB.println(".");

  // Example: Test the signal quality.
  // This returns a number between 0 and 5.
  // 2 or better is preferred.
  err = modem.getSignalQuality(signalQuality);
  if (err != ISBD_SUCCESS)
  {
    SerialUSB.print("SignalQuality failed: error ");
    SerialUSB.println(err);
    return;
  }

  SerialUSB.print("On a scale of 0 to 5, signal quality is currently ");
  SerialUSB.print(signalQuality);
  SerialUSB.println(".");

  // Send the message
  SerialUSB.print("Trying to send the message.  This might take several minutes.\r\n");
  int str_len = message.length()+1;
  char toSend[str_len];
  message.toCharArray(toSend,str_len);
  err = modem.sendSBDText(toSend);
  if (err != ISBD_SUCCESS)
  {
    SerialUSB.print("sendSBDText failed: error ");
    SerialUSB.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      SerialUSB.println("Try again with a better view of the sky.");
  }

  else
  {
    SerialUSB.println("Hey, it worked!");
  }
}
  


void loop() {
  // put your main code here, to run repeatedly:

}

void collectSensorData(){
  message = DateTime() + ":";
  for(int i = 0; i < numSensors; i++){
    float temp = sensors[i].readTemperatureC();
    int humidity = (int) sensors[i].readHumidity();
    SerialUSB.println(temp);
    SerialUSB.println(humidity);
    message += " T:" + (String) temp + "/H:" + (String) humidity + ",";
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
