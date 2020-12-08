/**
 * Runs the High-Altitude Soil Montitoring System
 * 
 * @author Carson Fiechtner
 * 
 * EEPROM Addresses:
 *  0 - position in send interval
 */

#include <SHT1x.h>
#include <RTCZero.h>
RTCZero rtc;

////////////////////////////////////////////////
/* Date/Time constant setup */
////////////////////////////////////////////////
// Change these values to set the initial time
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 11;
////////////////////////////////////////////////
// Chanage these values to set the initial date
const byte day = 6;
const byte month = 23;
const byte year = 20;

// Send data every [sendInterval] days
const int sendInterval = 2;
// Character denoting the end of a day's readings
const char msgEnd = ';';
// # of days included in a RockBlock message
const int daysPerMsg = 2;

// Specify data and clock connections and instantiate SHT1x object
#define dataPin1  9
#define dataPin2  8
#define dataPin3  7
#define dataPin4  6
#define clockPin  5
#define sdChipSelect 4

// Specify chip select pin for SD card communication


#define IridiumSerial Serial1

SHT1x sht1(dataPin1, clockPin);
// SHT1x sht2(dataPin2, clockPin);
// SHT1x sht3(dataPin3, clockPin);
// SHT1x sht4(dataPin4, clockPin);

// SHT1x sensors [] = {sht1, sht2, sht3, sht4}; for testing purposes, only one sensor is hooked up.
SHT1x sensors [] = {sht1};
int numSensors = sizeof(sensors) / sizeof(sensors[0]);
#include <SPI.h>
#include <SD.h>
#include <IridiumSBD.h>

// File on SD card containing stored temperature data
File dataFile;
// File on SD card detailing where to start reading temperature data
File posFile;
// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial);

void setup() {
  rtc.begin(); // initialize RTC 24H format
  rtc.setTime(hours,minutes,seconds);
  rtc.setDate(day,month,year);
  SerialUSB.begin(9600);
  delay(10000);
  collectSensorData();
  startSD();
  //clearFiles();
  initializeRB();
  sendData();
  

  SerialUSB.println("Done!");
}

void clearFiles() {
  SD.remove("data.txt");
  SD.remove("position.txt");
  posFile = SD.open("position.txt", FILE_WRITE);
  posFile.write('0');
  posFile.close();
}

void initializeRB() {
  // Start the serial port connected to the satellite modem
  IridiumSerial.begin(19200);

  // Begin satellite modem operation
  SerialUSB.println("Starting modem...");
  int err = modem.begin();
  if (err != ISBD_SUCCESS)
  {
    SerialUSB.print("Begin failed: error ");
    SerialUSB.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      SerialUSB.println("No modem detected: check wiring.");
    return;
  }
}

bool sendMessage(const char* msg) {
  SerialUSB.print("Trying to send the message.  This might take several minutes.\r\n");
  int err = modem.sendSBDText(msg);
  if (err != ISBD_SUCCESS)
  {
    SerialUSB.print("sendSBDText failed: error ");
    SerialUSB.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      //Serial.println("Try again with a better view of the sky.");
    return false;
  }
  return true;
}

void sendData() {
  // Get a reference to current position in data file
  int pos = getPos(), lastPos = pos;
  bool trySend = true, changed = false;
  
  // Open data file and move to current position.
  dataFile = SD.open("data.txt");
  dataFile.seek(pos);
  
  while(dataFile.available() && trySend){
    String curMsg = "";
    char c;
    int curdaysPerMsg = 0;
    lastPos = pos;
    while(dataFile.available() && curdaysPerMsg < daysPerMsg){
      c = dataFile.read();
      curMsg += c;
      pos++;
      if(c == msgEnd){
        curdaysPerMsg++;
      }
    }
    
    //Convert msg to char array for sending purposes
    int str_len = curMsg.length() + 1; 
    char char_array[str_len];
    curMsg.toCharArray(char_array, str_len);
    
    //Try to send message
    if(sendMessage(char_array)) {
      changed = true;
      //Serial.println("Sent!");
    } else {
      trySend = false;
      //Serial.println("Could not send");
    }
  }
  //Determine if all data was successfully sent
  bool allDataSent = (!dataFile.available() && trySend);
  dataFile.close();
  SerialUSB.println(allDataSent);
  SerialUSB.println(pos);
  //Reset send interval to pause send attempts
  if(allDataSent){
//    EEPROM.write(0,0);
    setPos(pos);
    SerialUSB.println(getPos());
  } else if(changed) {
    setPos(lastPos);
  }
}

void startSD(){
  SerialUSB.print("Initializing SD card...");
  SD.begin(sdChipSelect);
  if (!SD.begin(sdChipSelect)) {
    SerialUSB.println("initialization failed!");
    return;
  }
  SerialUSB.println("initialization done.");
}

void collectSensorData() {
  String measurement = collectDate() + ":";
  for(int i = 0; i < numSensors; i++){
    float temp = sensors[i].readTemperatureC();
    int humidity = (int) sensors[i].readHumidity();
    SerialUSB.println(temp);
    SerialUSB.println(humidity);
    //Convert and store readings
    measurement += " T:" + (String) temp + "/H:" + (String) humidity + ",";
  }
  measurement += msgEnd;
  dataFile = SD.open("data.txt", FILE_WRITE);
  dataFile.print(measurement);
  dataFile.close();
}

String collectDate() {
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

int getPos() {
  posFile = SD.open("position.txt");
  char result[7]; 
  uint8_t i = 0; // a counter
  char c;
  
  while (c != '\0' && i < sizeof(result) - 1) // while character isn't the string terminator
  {
   c = posFile.read(); // get next character from file
   result[i] = c; // store character in result
   i++; // increase counter
  }
  result[i] = '\0';

  int number = atoi(result); // convert result string to numeric value
  posFile.close();
  return number;
}

void setPos(int newPos) {
  SD.remove("position.txt");
  posFile = SD.open("position.txt", FILE_WRITE);
  String pos = (String) newPos;
  posFile.println(pos + '\0');
  posFile.close();
}

String makeItNice(String stringIn){
  //Inputs a 0 at the beginning of a string
  String  stringOut = "0" + stringIn;
  return stringOut;
}

void loop() {
}
