/*
 https://www.electronicwings.com/
  SD Card Interface code for ESP32
  SPI Pins of ESP32 SD card as follows:
  CS    = 5;
  MOSI  = 23;
  MISO  = 19;
  SCK   = 18; 
*/

#include <SPI.h>
#include <SD.h>
#include "WiFi.h"
#include <HardWareSerial.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>

// Create an instance of TinyGPS++
TinyGPSPlus gps;
// The serial connection to the GPS module
HardwareSerial MySerial(1);

#define RxPin 16
#define TxPin 17
#define baud 9600
#define SER_BUF 1024

const int maxNetworks = 20;     // Maximum number of networks to store
String ssidArray[maxNetworks];  // Array to store SSIDs
int networkCount = 0;

File myFile;
const int CS = 5;

void setup() {
  Serial.begin(9600);  // Set serial baud rate to 9600
  MySerial.setRxBufferSize(SER_BUF);
  MySerial.begin(baud, SERIAL_8N1, RxPin, TxPin);
  delay(500);
  while (!Serial) { ; }  // wait for serial port to connect. Needed for native USB port only
  Serial.println("Initializing SD card...");
  if (!SD.begin(CS)) {
    Serial.println("initialization failed!");
    return;
  }
  ReadConfig();
  Serial.println("initialization done.");
}

void loop() {
  String time = "";
  String date = "";
  String lat = "0.0";
  String lng = "0.0";

  while (MySerial.available() > 0) {
    // Feed the GPS data into the gps object
    gps.encode(MySerial.read());

    // Check if a new location is available
    // Extract latitude and longitude
    lat = String(gps.location.lat(), 6);
    lng = String(gps.location.lng(), 6);
    time = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
  }

  Serial.println("Scan Start");
  if (lat == "0.000000" && lng == "0.000000") {
    Serial.println("Unable to find location");
    delay(2000);
    return;
  }
  StaticJsonDocument<200> jsonDoc;
  Serial.println(lat + "#" + lng);
  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    for (int i = 0; i < n; ++i) {
       
      jsonDoc["date"] = gps.date.year() * 10000 + gps.date.month() * 100 + gps.date.day(); // YYYYMMDD
      jsonDoc["time"] = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
      jsonDoc["ssid"] = WiFi.SSID(i);
      jsonDoc["encryption"] = getEncryptionType(WiFi.encryptionType(i));
      jsonDoc["latitude"] = gps.location.lat();
      jsonDoc["longitude"] = gps.location.lng();
      jsonDoc["uploaded"] = false;

      ValidateAndWriteToSSIDFile("/ssid.txt", jsonString);
    }
  }

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  // Wait a bit before scanning again.
  delay(5000);
}


void WriteFile(const char* path,  StaticJsonDocument<200> jsonDoc) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(path, FILE_APPEND);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.printf("Appending to %s ", path);
    myFile.println(message);
    myFile.close();  // close the file
    Serial.println("completed.");
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening file ");
    Serial.println(path);
  }
}


void ReadFile(const char* path) {
  // open the file for reading:
  myFile = SD.open(path);
  if (myFile) {
    Serial.printf("Reading file from %s\n", path);
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();  // close the file:
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void ReadConfig() {
  // open the file for reading:
  myFile = SD.open("/config.txt");
  if (myFile) {
    Serial.println("Reading Config");
    // read from the file until there's nothing else in it:
    int i = 0;
    while (myFile.available()) {
      if (networkCount < maxNetworks) {
        String ssid = myFile.readStringUntil('\n');

        ssid.trim();
        ssidArray[networkCount] = ssid;
        networkCount++;
      } else {
        // If maxNetworks is reached, stop reading
        break;
      }
    }
    myFile.close();  // close the file:
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  Serial.println("Config loaded");
}



String getEncryptionType(int value) {
  switch (WiFi.encryptionType(value)) {
    case WIFI_AUTH_OPEN:
      return "open";
      break;
    case WIFI_AUTH_WEP:
      return "wep";
      break;
    case WIFI_AUTH_WPA_PSK:
      return "wpa";
      break;
    case WIFI_AUTH_WPA2_PSK:
      return "wpa";
      break;
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA+WPA2";
      break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-EAP";
      break;
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";
      break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return ("WPA2+WPA3");
      break;
    case WIFI_AUTH_WAPI_PSK:
      return "WAPI";
      break;
    default:
      return "unknown";
  }
}

void ValidateAndWriteToSSIDFile(const char* path, String message) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.

  myFile = SD.open(path);
  if (myFile) {
    // read from the file until there's nothing else in it:
    boolean found = false;
    while (myFile.available()) {
      String ssid = myFile.readStringUntil('\n');
      ssid.trim();
      message.trim();
      if (message.equals(ssid)) {
        found = true;
        break;
      }
    }

    myFile.close();

    if (!found) {
      myFile = SD.open(path, FILE_APPEND);
      if (myFile) {
        Serial.printf("Appending to %s ", path);
        myFile.println(message);
        myFile.close();  // close the file
        Serial.println("completed.");
      } else {
        Serial.println("error opening file ");
        Serial.println(path);
      }
    }
  }
}