#include <SoftwareSerial.h>;
SoftwareSerial mySerial(A4,A3);
char mystr[15] = "All Done!";
bool recievedFlag = false;
void setup(){
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  mySerial.begin(9600);
  Serial.begin(9600);
  Serial.write("What is the date?");
  mySerial.write("What is the date?");
}


void loop() {
    while((mySerial.available() > 0) && (recievedFlag == false)){
      mySerial.readBytes(mystr,15);
      Serial.println(mystr);
      recievedFlag = true;
      digitalWrite(13,HIGH);
    }

    if((mySerial.available()<=0)&&(recievedFlag == true)){
      delay(5000);
      sendMsg(mystr);
    }
}// end of loop

void sendMsg(String rtrStr){
  // initialize message to send back, can be any length, shorter the better
  int str_len = rtrStr.length() + 1;
  char toSend[str_len];
  rtrStr.toCharArray(toSend,str_len);
  delay(5);
  mySerial.write(toSend,str_len);
  Serial.write(toSend);
  delay(5);
}
