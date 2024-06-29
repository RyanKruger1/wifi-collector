#include <HardWareSerial.h>
#include <TinyGPSPlus.h>

// Create an instance of TinyGPS++
TinyGPSPlus gps;
// The serial connection to the GPS module
HardwareSerial MySerial(1);

#define RxPin   16
#define TxPin   17
#define baud    9600
#define SER_BUF 1024

void setup(){
  Serial.begin(9600);

  MySerial.setRxBufferSize(SER_BUF);
  MySerial.begin(baud,SERIAL_8N1,RxPin,TxPin);
}

void loop(){
while (MySerial.available() > 0) {
    // Feed the GPS data into the gps object
    gps.encode(MySerial.read());

    // Check if a new location is available
    if (gps.location.isUpdated()) {
      // Extract latitude and longitude
      double latitude = gps.location.lat();
      double longitude = gps.location.lng();

      // Print the latitude and longitude to the Serial Monitor
      Serial.print(F("Latitude: "));
      Serial.println(latitude, 6); // Print latitude with 6 decimal places
      Serial.print(F("Longitude: "));
      Serial.println(longitude, 6); // Print longitude with 6 decimal places
    }
  }
}