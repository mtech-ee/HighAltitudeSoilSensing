/********************************************************************************
 * Mini Ultra Pro HAST Alarm Code
 * Version 1.2.0
 * Date: 8-3-2020
 * Montana Technological University
 * Author: J. D. Ehlers
 * 
 * THIS IS TEST CODE, PLEASE SEE THE FINISHED VERSION FOR FULL APPLICATION
 * 
 * Code to collect control High Altitude Soil Testing system.
 * Collects soil data from SHT10 soil sensors, reads/writes data to SD card for storage,
 * manages power consumption and operation on PCB, keeps track of time and date and 
 * transmits soil data via email from RockBLOCK Mk 2 satellite modem. Alarm is set to 
 * go off every 30 seconds in order to take a measuremnt and write to SD card via .txt file.
 * 
 * Interacting Components:
 * 1. Mini Ultra Pro v3 (RocketScream), powered by 3.6 V cell source
 * 2. SHT10 Soil Sensor (Records temperature and humidity, this version uses 1 sensor)
 * 3. SD Card adapter (This version assumes that the SD card has a 5V regulator on it's board. Not worried about powering it.)
 * 4. RockBLOCK Mk2 Sattellite Modem (Used to send email of "Twitter length")
 * 5. Optoisolator (Used to isoalte RocketScream from RockBLOCK to avoid leakage current from trying to power either)
 * 6. 5V Regulator (Powers SD card adapter and RockBLOCK) 
 * 7. 3.3V Regulator (Powers SHT10 Soil Sensors and optoisolator)
 * 8. 3 X 3.6V Lithium-Thionyl D-cell size batteries
 * 
 * Important Notes: 
 * 1. Upon using the SerialFlash.sleep() function, the only function that the serial flash chip resonds to is SerialFlash.wakeup()
 * 2. Delay before USBDevice.attach is to prevent bricking and allows for update of code, DO NOT CHANGE
 * 
 * Revision Description
 * ======== ===========
 * 0.1.0 Initial creation, coppied over power management code from www.rocketscream.com
 * 1.1.0 Added functions from FullTest (still need to do full run down, make sure functional)
 * 1.1.1 Tested SD card and sensor scan functions, worked for taking a measurement every minute.
 * 1.2.0 Added RockBlock functions, need to setup watchdog for signal quality, as well as message length watchdog.
 * 
 ********************************************************************************/

#include <SerialFlash.h>
#include <RTCZero.h>
#include <SPI.h>
#include <SD.h>
#include <SHT1x.h>
#include <IridiumSBD.h>

#define IridiumSerial Serial1

IridiumSBD modem(IridiumSerial);

#define dataPin1 9
#define clockPin 5

#define chipSelect 4

File dataFile;

File posFile;


// ********CONSTANTS********
const int readingsPerMsg = 3;
const char msgEnd = ';';
int curDaysPassed = 0;
const int chipSelectPin = 4;

SHT1x sht1(dataPin1,clockPin);
SHT1x sensors[] = {sht1};
const int numSensors = sizeof(sensors) / sizeof(sensors[0]);

RTCZero rtc;

/* Change these values to set the current inital time */
const uint8_t seconds = 0;
const uint8_t minutes = 0;
const uint8_t hours = 12;

/* Change these values to set the current inital date */
const uint8_t day = 27;
const uint8_t month = 7;
const uint8_t year = 20;

// ********VARIABLES********
unsigned char pinNumber;
bool alarmVar = false;
int numHoursPassed = 0;
String message;

void setup(){
  // Switch unused pins as input and enabled built-in pullup
  for (pinNumber = 0; pinNumber < 23; pinNumber++){
    pinMode(pinNumber,INPUT_PULLUP);
  }

  // Unconsecutve pin-numbering from 23-32

  for (pinNumber = 32; pinNumber < 42; pinNumber ++){
    pinMode(pinNumber,INPUT_PULLUP);
  }

  pinMode(25,INPUT_PULLUP);
  pinMode(26,INPUT_PULLUP);

  // CS pin on RocketScream is 4
  SerialFlash.begin(chipSelectPin);
  SerialFlash.sleep();

  // ***** IMPORTANT DELAY FOR CODE UPLOAD BEFORE USB PORT DETACH DURING SLEEP *****
  delay(15000);                         // Delay is 15 seconds long

  // RTC initialization
  rtc.begin();                          // inititalizes rtc
  rtc.setTime(hours, minutes, seconds); // initializes rtc time
  rtc.setDate(day, month, year);        // inits rtc date

  // RTC alarm setting on every 15 s resulting in 1 minute sleep period
  rtc.setAlarmSeconds(15);              // Initial alarm goes off in 15 seconds
  rtc.enableAlarm(rtc.MATCH_SS);        // Once alarm goes off, in 1 minute the alarm will go off after going into sleep mode
  rtc.attachInterrupt(alarmMatch);      // attatches alarm interrupt to alarmMatch

  USBDevice.detach();                   // detaches USB interface
  rtc.standbyMode();                    // rtc goes into standby mode
}



void loop() {
  // Initialize USB and attach to host (not required if not in use)

    if (alarmVar == true){
        USBDevice.init();                     // initializes USB interface
        USBDevice.attach();                   // attaches USB interface
        delay(3000);                          // delay for reseting serial monitor.
        SerialUSB.begin(9600);
        SerialUSB.println(curDaysPassed);
        collectSensorData();
        initSD();
        writeSD();
        numHoursPassed += 1;
    }
    
    if (curDaysPassed == 7){
        initRB();
        sendData();
        alarmVar = false;
        delay(5000);                          // delay for testing. 
        curDaysPassed = 0;
    }
    
    if (numHoursPassed == 24){
      curDaysPassed += 1;
      numHoursPassed = 0;
    }
      
      USBDevice.detach();                   // Detach USB from host (not required if not in use)
      rtc.standbyMode();
  
}

// Don't put anything in alarmMatch, unless to increment/reset counters
void alarmMatch(){
  alarmVar = true;
}

void collectSensorData(){
  message = DateTime();
  for(int i = 0; i < numSensors; i++){
    float temp = sensors[i].readTemperatureC();
    int humidity = (int) sensors[i].readHumidity();
    SerialUSB.println(temp);
    SerialUSB.println(humidity);
    message += (String) temp + (String) humidity + ",";
  }
  message += msgEnd;
}

String DateTime(){
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

  String dateNow = stringDay + stringMonth + stringYear + stringHour + stringMin;
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
  //result[i] = '\0';

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
    int curReadingsPerMsg = 0;
    lastPos = pos;
    while(dataFile.available() && curReadingsPerMsg < readingsPerMsg){
      c = dataFile.read();
      curMsg += c;
      pos++;
      if(c == msgEnd){
        curReadingsPerMsg++;
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
