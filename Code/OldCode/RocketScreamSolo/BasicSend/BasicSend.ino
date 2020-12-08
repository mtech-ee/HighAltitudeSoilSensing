#include <IridiumSBD.h>

/*
 * BasicSend
 * 
 * This sketch sends a "Hello, world!" message from the satellite modem.
 * If you have activated your account and have credits, this message
 * should arrive at the endpoints you have configured (email address or
 * HTTP POST).
 * 
 * Assumptions
 * 
 * The sketch assumes an Arduino Mega or other Arduino-like device with
 * multiple HardwareSerial ports.  It assumes the satellite modem is
 * connected to Serial1.  Change this as needed.  SoftwareSerial on an Uno
 * works fine as well.
 */

#define IridiumSerial Serial1
#define DIAGNOSTICS false // Change this to see diagnostics
#define RB_PWR   11
#define COMM_PWR 13

const int RB_BAUD = 19200;
const int SERIAL_BAUD = 9600;
String message;


// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial);

void setup()
{
  
  pinMode(RB_PWR, OUTPUT);
  pinMode(COMM_PWR, OUTPUT);
  digitalWrite(RB_PWR,HIGH);
  digitalWrite(COMM_PWR,HIGH);
  
  int signalQuality = -1;
  int err;
  
  // Start the console serial port
  delay(5000);
  SerialUSB.begin(115200);
  delay(5000);
  while (!SerialUSB){
    SerialUSB.println("Help I'm stuck");
  }

  // Start the serial port connected to the satellite modem
  IridiumSerial.begin(19200);

  // Begin satellite modem operation
  SerialUSB.println("Starting modem...");
  err = modem.begin();
  if (err != ISBD_SUCCESS)
  {
    SerialUSB.print("Begin failed: error ");
    SerialUSB.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      SerialUSB.println("No modem detected: check wiring.");
    return;
  }

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
  //SerialUSB.print("Trying to send the message.  This might take several minutes.\r\n");
  //err = modem.sendSBDText("Hello, world!");
  SerialUSB.println("We would be sending a message here, but not in this economy.");
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

void loop()
{
}

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  SerialUSB.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  SerialUSB.write(c);
}
#endif
