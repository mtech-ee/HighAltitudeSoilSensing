////////////////////////////////////////////////////////////////////
/* Global Variable Block / Include Block
*
*
 */
#include <SoftwareSerial.h>;
#include <SD.h>
// #include <SHT1x.h>

// Specify data and clock connections and instantiate SHT1x object
// #define dataPin1  9
// #define dataPin2  8
// #define dataPin3  7
// #define dataPin4  6
// #define clockPin  5

// SHT1x sht1(dataPin1, clockPin);
// SHT1x sht2(dataPin2, clockPin);
// SHT1x sht3(dataPin3, clockPin);
// SHT1x sht4(dataPin4, clockPin);

// SHT1x sensors [] = {sht1,sht2,sht3,sht4};
// int numSensors = sizeof(sensors) / sizeof(sensors[0]);

SoftwareSerial RocketSerial(A4,A3);
char dateMsg[15];
char handshake = "What is the date?";
char mystr[15];
bool recievedFlag = false;
bool allClearSentFlag = false;
int RocketRate = 9600;
////////////////////////////////////////////////////////////////////

void setup(){
  char dateStamp;
  Serial.begin(RocketRate);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  RocketSerial.begin(RocketRate);
  delay(5000);
  sendHandshake();
  dateStamp = readMsg();
  Serial.println("Today's date is:");
  Serial.println(dateStamp);
  allClear();
}


void allClear(){
     while(allClearSentFlag == false){
        if(RocketSerial.available()<=0){
          delay(5000);
          sendMsg(dateMsg);
          Serial.println(dateMsg);
          digitalWrite(13,HIGH);
          allClearSentFlag = true;
        }// end of if
     }// end of while
}// end of allClear()

void sendMsg(String rtrStr){
  int str_len = rtrStr.length() + 1;
  char toSend[str_len];
  Serial.println("Message Sent:");
  Serial.println(toSend);
  rtrStr.toCharArray(toSend,str_len);
  delay(3000);
  RocketSerial.write(toSend,str_len);
  Serial.write(toSend);
  delay(5);
}

char readMsg(){
  while((RocketSerial.available() > 0) && (recievedFlag == false)){
    RocketSerial.readBytes(dateMsg,15);
    delay(5000);
    recievedFlag = true;
  }// end while
  return dateMsg;
}// end readMsg()

void loop() {
 
}// end of loop

void sendHandshake(){
  RocketSerial.write("What is the date?");
  delay(5000);
  Serial.println("What is the date?");
  
}
