/*
 * HAST PCB Testing Program
 * 
 * Tests board for operational parameters
 * 
 * Jamison D. Ehlers, Undergraduate Student
 * Electrical Engineering Department, Montana Technological University
 * jehlers@mtech.edu
 */
// Include Required Packages
#include <SerialFlash.h>	// Flash memory control
#include <RTCZero.h>		// Low Power
#include <SPI.h>   			// SPI communication
#include <SD.h>      		// SD card control
#include <SHT1x.h>    		// Soil temperature and moisture sensor
#include <IridiumSBD.h>     // Satellite communication


#define IridiumSerial Serial1     // Define UART line with RockBlock
IridiumSBD modem(IridiumSerial);  // Declare UART line with RockBlock

// Constants for communication

/*
 * SENSOR PWR:      GPIO10
 * RB PWR:          GPIO11
 * SD PWR:          GPIO12
 * RB COMM PWR:     GPIO13
 * SENSOR 1:        GPIO9
 * SENSOR 2:        GPIO8
 * SENSOR 3:        GPIO7
 * SENSOR 4:        GPIO6
 * CS for Flash:    GPIO4
 * CLOCK (SENSOR):  GPIO5
 */
 
#define DATA1_PIN   9   // Data line for sensor 1
#define DATA2_PIN   8 	// Data line for sensor 2
#define DATA3_PIN   7		// Data line for sensor 3
#define DATA4_PIN   6		// Data line for sensor 4
       
#define CLK_PIN     5		// Clock line for soil sensors
#define SENSE_PWR   10	// Chip select pin for sensor 3.3V regulator
#define RB_PWR      11	// Chip select pin for RockBlock 5V regulator
#define SD_PWR      12	// Chip select pin for SD card 5V regulator
#define COMM_PWR    13	// Chip select pin for optocoupler 3.3V regulator

File dataFile;				  // File on SD card for storing soil readings
File posFile;				    // File on SD card for storing current position in dataFile

int READINGS_PER_MSG = 3;	    // Ammount of readings sent with each message, uncoded = 28 bytes, coded = 14 bytes maximum of 50 bytes per credit, 360 bytes per message
const char MSG_END = ';';	    // Delimeter of each soil reading
const int CS_PIN = 4;		      // Chip select pin for SD card
const int SERIAL_BAUD = 9600;	// Baud rate for Serial monitor line to host PC
const int RB_BAUD = 19200;		// Baud rate for UART line to RockBlock
const int SCANS_PER_DAY = 24;	// Number of scans per day

// SHT Sensor objects for each sensor, attaches respective data lines to each pin/sensor
SHT1x SHT1(DATA1_PIN, CLK_PIN);
SHT1x SHT2(DATA2_PIN, CLK_PIN);
SHT1x SHT3(DATA3_PIN, CLK_PIN);
SHT1x SHT4(DATA4_PIN, CLK_PIN);

SHT1x SENSORS[] = {SHT1,SHT2,SHT3,SHT4};                    // Array of sensor objects
const int NUM_SENSE = sizeof(SENSORS)/sizeof(SENSORS[0]);   // Number of sensors in use

RTCZero rtc;				                                        // Real time clock library object


const uint8_t SEC = 0;    	// Starting seconds for RTC
const uint8_t MIN = 0;    	// Starting minutes for RTC
const uint8_t HOUR = 12;  	// Starting hours for RTC

const uint8_t DAY = 27;   	// Starting day for RTC
const uint8_t MON = 8;    	// Starting minutes for RTC
const uint8_t YEAR = 20;  	// Starting hours for RTC

unsigned char PIN_NUMBER;	  // Keeps track of which pin is being disabled in pinOff
bool ALARM_VAR = false;		  // Flg that enables or disables operation of forever loop
int SCANS_TAKEN = 0;		    // Current number of scans taken
int SEND_ATTEMPTS = 0;      // Current number of failed transmission attempts
String message;             // Buffer for sensor readings
int signalQuality = -1;     // Initialization of signal quality

void setup() {
  delay(5000); 				              // Safety delay
  pinoff();                         // Turn off all unneeded pins
  SerialFlash.begin(CS_PIN);
  SerialFlash.sleep();
  delay(15000); 			              // Safety delay  2
  rtc.begin();					            // Begin real time clock
  rtc.setTime(HOUR, MIN, SEC);      // Set real time clock time
  rtc.setDate(DAY,MON,YEAR);	      // Set real time clock dateNow
  rtc.setAlarmMinutes(5);				    // Set sensor time interval
  rtc.enableAlarm(rtc.MATCH_MMSS);	// Enable alarm when time matches 
  rtc.attachInterrupt(alarmMatch);	// Attach interrupt to alarm matche
  delay(500); 						          // Safety delay 3
  USBDevice.detach();				        // Detach USB from host computer
  rtc.standbyMode();				        // Put system in standby mode
 
}

void loop() {
  if (ALARM_VAR == true){           // May not be needed
    USBDevice.init();				        // Initialize the USB connection to host
    USBDevice.attach();				      // Attach USB to host
    delay(3000);					          // Safety delay for 
    SerialUSB.begin(SERIAL_BAUD);	  // Begin serial debugging line to host
    String bootUpMsg = "Current number of readings taken since last message: " + String(SCANS_TAKEN); // status message, not needed for final
    SerialUSB.println(bootUpMsg);
    scan();							            // Take a soil temperature and moisture scan
    initSD();						            // Initialize SD card for write
    writeSD();						          // Write soil measurements to SD card
    
    SCANS_TAKEN += 1;				        // Increment payload of current number of scans
    if (SCANS_TAKEN >= READINGS_PER_MSG){	
      initRB();						          // Initialize RockBlock and check signal quality  
      if (signalQuality > 3){		
        sendData();					        // Send data if signal quality is at least 3 out of 5
        SCANS_TAKEN = 0;			      // Clear SCANS_TAKEN if sent successfully
		SEND_ATTEMPTS = 0;			        // Clear SEND_ATTEMPTS if sent successfully
      }
      else{
        SEND_ATTEMPTS++;			      // Increment SEND_ATTEMPTS if signal quality is low
        if(SEND_ATTEMPTS > 3){
          READINGS_PER_MSG += SEND_ATTEMPTS;	// If unsuccessfully sent more than three time, increment the 
        }   
        else{
          READINGS_PER_MSG = 3;		  // Reset if message sent successfully
        }
      }
    }
    ALARM_VAR = false;				      // Reset alarm
    pinoff();
    USBDevice.detach();				      // Detach USB from host			
    rtc.standbyMode();				      // Enter standby mode
  }
}

void alarmMatch(){
  ALARM_VAR = true;					// Trigger alarm
}

void pinoff(){
/*	Each pin needs to be set as an input during sleep mode, 
 *	so as to meet our 20 uA sleep mode requirement.
 */
  for(PIN_NUMBER = 0; PIN_NUMBER <23; PIN_NUMBER++){
    pinMode(PIN_NUMBER,INPUT_PULLUP); 
  }
  // skip pins 24,27-31, as they do not exist
  for(PIN_NUMBER = 32; PIN_NUMBER <42; PIN_NUMBER++){
    pinMode(PIN_NUMBER,INPUT_PULLUP); 
  }
  
  pinMode(25,INPUT_PULLUP);
  pinMode(26,INPUT_PULLUP);
}

void scan(){
/*  Take a temperature and moisture reading from the soil
 *  load into message to write to SD card 
 */
  pinMode(SENSE_PWR,OUTPUT);	                            // Set chip select pin for sensor regulator as output
  digitalWrite(SENSE_PWR,HIGH);	                          // Pull regulator chip select high
  message = DateTime();			                              // Add time stamp to the message string
  for(int i = 0; i < NUM_SENSE; i++){
    float temp = SENSORS[i].readTemperatureC();	          // Take temperature reading
    int humidity = SENSORS[i].readHumidity();	            // Take moisture reading
    message += (String) temp + (String) humidity + ",";	  // Load current sensor reading into the message string
  }
  message += MSG_END;			                                // Add message delimeter
}

String DateTime(){
  
/*	Collect time stamp and output as a string
 *	Also uses makeItNice to make time stamp regular length
 */
 
  int dayNow = rtc.getDay();		  // Collect day from real time clock
  int monthNow = rtc.getMonth();	// Collect month from real time clock
  int yearNow = rtc.getYear();		// Collect year from real time clock

  int secondNow = rtc.getSeconds();	// Collect seconds from real time clock
  int minuteNow = rtc.getMinutes();	// Collect minutes from real time clock
  int hourNow = rtc.getHours();		  // Collect hours from real time clock

  String stringDay = String(dayNow,DEC);	  // Convert date values from integers to a string
  String stringMonth = String(monthNow,DEC);
  String stringYear = String(yearNow,DEC);

  String stringSec = String(secondNow,DEC);	// Convert time values from integers to a string
  String stringMin = String(minuteNow,DEC);
  String stringHour = String(hourNow,DEC);
	
// If time and date values are less than 10, adds zero to beginning
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

  String dateNow = stringDay + stringMonth + stringYear + stringHour + stringMin;	// Combines time stamp into one string
  return dateNow;					// Outputs time stamp as string
}

String makeItNice(String stringIn){
  // Adds a 0 to the beginning of a string
  String  stringOut = "0" + stringIn;
  return stringOut;
}

void initSD(){
/* Pulls the chip select pin high 
 * waits for verification from SD card adapter
 */
  SerialUSB.print("INITIALIZING SD CARD...");
  pinMode(SD_PWR,OUTPUT);
  digitalWrite(SD_PWR,HIGH);
  if(!SD.begin(CS_PIN)){
    SerialUSB.println("INIT FAILED, GOING TO SLEEP");
    //while(1);
  }
  SerialUSB.print("SUCCESS");
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

  else{
    SerialUSB.println("ERROR OPENING data.txt");
  }
}

void writeSD(){
  dataFile = SD.open("data.txt", FILE_WRITE);
  if(dataFile){
    SerialUSB.println("WRITING TO data.txt...");
    dataFile.println(message);
    dataFile.close();
    SerialUSB.print("DONE.");
  }
  else{
    SerialUSB.println("ERROR OPENING data.txt");
  }
}

int getPos(){
  posFile = SD.open("position.txt");
  char result[7];
  uint8_t i = 0;
  char c;

  while(c != '\0' && i < sizeof(result) - 1){
    c = posFile.read();
    result[i] = c;
    i++;
  }
  result[i] = '\0';

  int number = atoi(result);
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
  pinMode(RB_PWR,OUTPUT);
  digitalWrite(RB_PWR,HIGH);
  
  pinMode(COMM_PWR,OUTPUT);
  digitalWrite(COMM_PWR,HIGH);
  
  IridiumSerial.begin(19200);
  
  int err;
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
    // Get Signal Quality
    // Returns a number between 0 and 5.
    // 3 or better preferred.
    // Saves value to global variable signalQuality
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

bool sendMessage(char* msg) {
  String msg2convert = String(msg);
  
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
    SerialUSB.println("Message sent successfully");
    SerialUSB.println("Message Sent: ");
    SerialUSB.print(msg);
  }
  return true;
}

bool sendData() {
  int pos = getPos(), lastPos = pos;   // Get a reference to current position in data file
  bool trySend = true, changed = false;
  
  // Open data file and move to current position.
  dataFile = SD.open("data.txt");
  dataFile.seek(pos);
  
  while(dataFile.available() && trySend){
    String curMsg = "";
    char c;
    int curdaysPerMsg = 0;
    lastPos = pos;
    while(dataFile.available() && curdaysPerMsg < READINGS_PER_MSG){
      c = dataFile.read();
      curMsg += c;
      pos++;
      if(c == MSG_END){
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
      //trySend = false;  Testing setpos, uncomment when done
      changed = true;
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
    setPos(pos);
    SerialUSB.println(getPos());
  } else if(changed) {
    setPos(lastPos);
  }
  return true;
}
