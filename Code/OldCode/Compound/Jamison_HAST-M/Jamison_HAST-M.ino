

/**
 * Runs the High-Altitude Soil Montitoring System
 * 
 * @author Carson Fiechtner
 * 
 * @edited Jamison Ehlers 6/9/2020
 *    May have memory stability problems, need to look into cleaning up code
 * EEPROM Addresses:
 *  0 - position in send interval
 */

#include <SHT1x.h>
//#include <SoftwareSerial.h>
// Specify Rx and Tx pins for Arduino/RocketScream connection
//SoftwareSerial RocketScreamSerial(A4,A3); // Rx, Tx
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

#define IridiumSerial Serial

SHT1x sht1(dataPin1, clockPin);
SHT1x sht2(dataPin2, clockPin);
SHT1x sht3(dataPin3, clockPin);
SHT1x sht4(dataPin4, clockPin);

SHT1x sensors [] = {sht1, sht2, sht3, sht4};
int numSensors = sizeof(sensors) / sizeof(sensors[0]);

//#include <EEPROM.h>
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
  Serial.begin(38400);
  RocketScreamSerial.begin(38400);
  char dateStamp = dateTime();
  
  initializeSD();
//  clearFiles();
//  EEPROM.write(0,0);
  collectSensorData(dateStamp);
  EEPROM.write(0, EEPROM.read(0)+1);
  if(EEPROM.read(0) % sendInterval == 0){
    initializeRB();
    sendData();
  }

  Serial.println("Done!");
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
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
  Serial.println("Starting modem...");
  int err = modem.begin();
  if (err != ISBD_SUCCESS)
  {
    Serial.print("Begin failed: error ");
    Serial.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      Serial.println("No modem detected: check wiring.");
    return;
  }
}

bool sendMessage(const char* msg) {
  Serial.print("Trying to send the message.  This might take several minutes.\r\n");
  int err = modem.sendSBDText(msg);
  if (err != ISBD_SUCCESS)
  {
    Serial.print("sendSBDText failed: error ");
    Serial.println(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      Serial.println("Try again with a better view of the sky.");
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
      Serial.println("Sent!");
    } else {
      trySend = false;
      Serial.println("Could not send");
    }
  }
  //Determine if all data was successfully sent
  bool allDataSent = (!dataFile.available() && trySend);
  dataFile.close();
  Serial.println(allDataSent);
  Serial.println(pos);
  //Reset send interval to pause send attempts
  if(allDataSent){
    EEPROM.write(0,0);
    setPos(pos);
    Serial.println(getPos());
  } else if(changed) {
    setPos(lastPos);
  }
}

void initializeSD(){
  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void collectSensorData(char dateStamp) {
  String measurement = dateStamp + ":";
  for(int i = 0; i < numSensors; i++){
    float temp = sensors[i].readTemperatureC();
    int humidity = (int) sensors[i].readHumidity();
    Serial.println(temp);
    Serial.println(humidity);
    //Convert and store readings
    measurement += " T:" + (String) temp + "/H:" + (String) humidity + ",";
  }
  measurement += msgEnd;
  dataFile = SD.open("data.txt", FILE_WRITE);
  dataFile.print(measurement);
  dataFile.close();
}

char dateTime() {
  char dateMsg[15];
  bool recievedFlag = false;
  //RocketScreamSerial.begin(38400);
  delay(3000);
//  RocketScreamSerial.write("What is the date?");
  while((recievedFlag == false)){
    RocketScreamSerial.readBytes(dateMsg,15);
    delay(3000);
    Serial.println(dateMsg);
  }// end while
  // String dateTimeString = dateMsg;
  //Serial.println(dateMsg);
  return dateMsg;
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



void loop() {
}
