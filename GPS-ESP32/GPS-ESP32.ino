#include <HardWareSerial.h>
#include <TinyGPSPlus.h>
#include <SPI.h>
#include <SD.h>

// Create an instance of TinyGPS++
TinyGPSPlus gps;
// The serial connection to the GPS module
HardwareSerial MySerial(1);

#define RxPin   16
#define TxPin   17
#define baud    9600
#define SER_BUF 1024

File myFile;
const int CS = 5;
const int counter = 0 ;

void setup(){
  Serial.begin(9600);

  MySerial.setRxBufferSize(SER_BUF);
  MySerial.begin(baud,SERIAL_8N1,RxPin,TxPin);

  delay(500);
  // while (!Serial) { ; }  // wait for serial port to connect. Needed for native USB port only
  // Serial.println("Initializing SD card...");
  // if (!SD.begin(CS)) {
  //   Serial.println("initialization failed!");
  //   return;
  // }
  // Serial.println("initialization done.");
}

void loop(){
while (MySerial.available() > 0) {
    // Feed the GPS data into the gps object
    gps.encode(MySerial.read());

    // Check if a new location is available
    if (gps.location.isUpdated()) {
      // Extract latitude and longitude
      String latitude = String(gps.location.lat());
      String longitude = String(gps.location.lng());
      String time = String(gps.date.year())+'-'+String(gps.date.month())+'-'+String(gps.date.day())+" "+String(gps.time.hour())+":"+String(gps.time.minute())+":"+String(gps.time.second());

      // Print the latitude and longitude to the Serial Monitor
      Serial.println(time);
      Serial.println(latitude);
      Serial.println(longitude);
      WriteFile("/values.txt", time.c_str());

    }
  }
}

void WriteFile(const char * path, const char * message){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(path, FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.printf("Writing to %s ", path);
    myFile.println(message);
    myFile.close(); // close the file:
    Serial.println("completed.");
  } 
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening file ");
    Serial.println(path);
  }
}


void ReadFile(const char * path){
  // open the file for reading:
  myFile = SD.open(path);
  if (myFile) {
     Serial.printf("Reading file from %s\n", path);
     // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close(); // close the file:
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}