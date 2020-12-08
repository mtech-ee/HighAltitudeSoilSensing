 #include <SoftwareSerial.h>;
SoftwareSerial mySerial(A4,A3);
char mystr[15];
bool recievedFlag = false;
char str2cmp[15] = "09/01/63 07:00";
void setup(){
  
  
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  mySerial.begin(9600);
  Serial.begin(9600);

  }
  
void loop(){
  
  if (mySerial.available()){
      mySerial.readBytes(mystr,15);
      recievedFlag = true;
      Serial.println(mystr);
      if(strcmp(mystr,str2cmp) == 0){
        digitalWrite(13,HIGH);
        Serial.println("Match Found");
      }
  }// recieve date from RocketScream
  
  if ((mySerial.available()<=0)&&(recievedFlag == true)){
    delay(5000);
      sendMsg(mystr);
     //recievedFlag = false;
  }// send byte to indicate ready to turn off
}

// send a message back to the RocketScream to indicate end of task
void sendMsg(String rtrStr){
  // initialize message to send back, can be any length, shorter the better
  //String rtrStr = "All Done!";
  int str_len = rtrStr.length() + 1;
  char toSend[str_len];
  rtrStr.toCharArray(toSend,str_len);
  delay(5);
  mySerial.write(toSend,str_len);
  Serial.write(toSend);
  delay(5);
}
