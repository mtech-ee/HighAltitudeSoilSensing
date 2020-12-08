/***SENSORS***/
#include <SHT1x.h>

#define dataPin1 7
#define clockPin 5

SHT1x sht1(dataPin1,clockPin);

SHT1x sensors[] = {sht1};
int numSensors = sizeof(sensors) / sizeof(sensors[0]);
/*************/

/***RTC***/
#include <RTCZero.h>
RTCZero rtc;

// Change these values to set the initial time
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 7;
// Chanage these values to set the initial date
const byte day = 7;
const byte month = 2;
const byte year = 16;
/*********/

/***SD/SPI***/
#include <SPI.h>
#include <SD.h>

#define chipSelect 4

File dataFile;
File posFile;
/************/

/***ROCKBLOCK***/
#include <IridiumSBD.h>

#define IridiumSerial Serial1

IridiumSBD modem(IridiumSerial);

/***GLOBAL***/
String message;
const char msgEnd = ';';
const int daysPerMsg = 2;
String curMsg = "";
/************/


/////////////////////////
// BEGIN MAIN FUNCTION //
/////////////////////////

void setup(){
  rtc.begin();
  rtc.setTime(hours,minutes,seconds);
  rtc.setDate(day,month,year);
  SerialUSB.begin(9600);
  pinMode(11,OUTPUT);
  digitalWrite(11,HIGH);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH);
  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);
  pinMode(6,OUTPUT);
  digitalWrite(6,HIGH);
  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);
  pinMode(9,OUTPUT);
  digitalWrite(9,HIGH);
  
  while(!SerialUSB){
    ;// Wait for USB connection to finish init
  }
  delay(5000); // Delay here for debugging purposes

    
  
  collectSensorData();
  initSD();
  writeSD();
  initRB();
  sendData();
}
///////////////////////
// END MAIN FUNCTION //
///////////////////////


////////////////////////////
// BEGIN FUNCTION SECTION //
////////////////////////////
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

void initSD(){
  SerialUSB.print("Initializing SD card...");
  if(!SD.begin(chipSelect)){
    SerialUSB.println("initialization failed!");
    while(1);
  }
  SerialUSB.println("initialization done.");
}

void readSD(){
  dataFile = SD.open("data.txt");
  if(dataFile){
    SerialUSB.println("data.txt");
    while(dataFile.available()){
      SerialUSB.write(dataFile.read());
    }
    dataFile.close();
  }
  else {
    SerialUSB.println("error opening data.txt");
  }
}

void writeSD(){
  dataFile = SD.open("data.txt",FILE_WRITE);
  if(dataFile){
    SerialUSB.print("Writing to data.txt...");
    dataFile.println(message);
    dataFile.close();
    SerialUSB.println("done.");
  }
  else{
    SerialUSB.println("error opening data.txt");
  }
}

int getPos(){
  posFile = SD.open("position.txt");
  char result[7];
  uint8_t i = 0; // counter
  char c;

   while (c != '\0' && i < sizeof(result) - 1){ 
    //while character isn't the string terminator
    c = posFile.read(); // get next character from file
    result[i] = c; // store character in result
    i++; // increase counter
  }
  result[i] = '\0';

  int number = atoi(result); // convert result string to numeric value
  posFile.close();
  return number;
}

void setPos(int newPos){
  SD.remove("position.txt");
  posFile = SD.open("position.txt", FILE_WRITE);
  String pos = (String) newPos;
  posFile.println(pos + '\0');
  posFile.close();
}

void initRB() {
  // Start the serial port connected to the satellite modem
  IridiumSerial.begin(19200);
  int err;
  int signalQuality = -1;
  // Begin satellite modem operation
  SerialUSB.println("Starting modem...");
  err = modem.begin();
  if (err != ISBD_SUCCESS){
    SerialUSB.print("Begin failed: error ");
    SerialUSB.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      SerialUSB.println("No modem detected: check wiring.");
    return;
  }
  else{
    // Get Firmware Version
    char version[12];
    err = modem.getFirmwareVersion(version,sizeof(version));
    if (err != ISBD_SUCCESS){
      SerialUSB.print("FirmwareVersion failed: error ");
      SerialUSB.println(err);
      return;
    }
    SerialUSB.print("Firmware Version is ");
    SerialUSB.print(version);
    SerialUSB.println(".");

    // Get Signal Quality
    // Returns a number between 0 and 5.
    // 2 or better preferred.
    err = modem.getSignalQuality(signalQuality);
    if (err!= ISBD_SUCCESS){
      SerialUSB.print("SignalQuality failed: error ");
      SerialUSB.println(err);
      return;
    }
    SerialUSB.print("On a scale of 0 to 5, signal quality is currently ");
    SerialUSB.print(signalQuality);
    SerialUSB.println(".");
    
  }
}

bool sendMessage(const char* msg) {
  SerialUSB.print("Trying to send the message.  This might take several minutes.\r\n");
  int err = modem.sendSBDText(msg);
  if (err != ISBD_SUCCESS){
    SerialUSB.print("sendSBDText failed: error ");
    SerialUSB.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT){
      SerialUSB.println("Try again with a better view of the sky.");
    return false;
    }
  }
  else{
    SerialUSB.println("Hey it worked!");
    SerialUSB.println("Message Sent: ");
    SerialUSB.print(msg);
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
      SerialUSB.println("Sent!");
    } else {
      trySend = false;
      SerialUSB.println("Could not send");
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

//////////////////////////
// END FUNCTION SECTION //
//////////////////////////

// No need for loop yet, just testing to see if SD card will work with RockBlock
void loop(){
}
