#include <SPI.h>
#include <SD.h>

#define chipSelect 4

File dataFile;

File posFile;

const int daysPerMsg = 4;
const char msgEnd = ';';
void setup() {
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH);
  SerialUSB.begin(9600);
  delay(10000);
  initializeSD();
  readSD();
}

void loop() {
}

void initializeSD(){
  SerialUSB.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    SerialUSB.println("initialization failed!");
    while(1);
  }
  SerialUSB.println("initialization done.");
}

int getPos(){
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

void setPos(int newPos){
  SD.remove("position.txt");
  posFile = SD.open("position.txt", FILE_WRITE);
  String pos = (String) newPos;
  posFile.println(pos + '\0');
  posFile.close();
}

void readSD() {
  // Get a reference to current position in data file
  int pos = getPos(), lastPos = pos;
  bool tryPrint = true, changed = false;
  
  // Open data file and move to current position.
  dataFile = SD.open("data.txt");
  dataFile.seek(pos);
  
  while(dataFile.available() && tryPrint){
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
    
    //Try to print message
    if(sendMessage(char_array)) {
      changed = true;
      SerialUSB.println(char_array);
      SerialUSB.println("Done!");
    } else {
      tryPrint = false;
      SerialUSB.println("Could not send");
    }
  }
  //Determine if all data was successfully sent
  bool allDataSent = (!dataFile.available() && tryPrint);
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

bool sendMessage(const char* msg){
  SerialUSB.println("Simmulating message sending.....");
  delay(5000);
  return true;
}
