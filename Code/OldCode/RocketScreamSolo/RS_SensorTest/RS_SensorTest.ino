//#include <IridiumSBD.h>
//#define IridiumSerial Serial
#include <SHT1x.h>
const char msgEnd = ';';

#define dataPin  2
#define clockPin 3

SHT1x sht1x(dataPin, clockPin);
SHT1x sensors [] = {sht1x};
int numSensors = sizeof(sensors) / sizeof(sensors[0]);

void setup() {
  SerialUSB.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  collectSensorData();
}

void collectSensorData() {
  String measurement = ":";
  float temp = sht1x.readTemperatureC();
  int humidity = sht1x.readHumidity();
  SerialUSB.println(temp);
  SerialUSB.println(humidity);
  //Convert and store readings
  measurement += " T:" + (String) temp + "/H:" + (String) humidity + ",";
  measurement += msgEnd;
}
