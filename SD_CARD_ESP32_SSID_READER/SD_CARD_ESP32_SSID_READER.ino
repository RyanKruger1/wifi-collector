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
const int maxNetworks = 20;     // Maximum number of networks to store
String ssidArray[maxNetworks];  // Array to store SSIDs
int networkCount = 0;

File myFile;
const int CS = 5;

void setup() {
  Serial.begin(9600);  // Set serial baud rate to 9600
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
  ReadFile("/ssid.txt");
  Serial.println("Scan start");

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    for (int i = 0; i < n; ++i) {

      String currentSSID = WiFi.SSID(i) + "#" + getEncryptionType(i);
      ValidateAndWriteToSSIDFile("/ssid.txt", currentSSID.c_str());
    }
  }

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();

  // Wait a bit before scanning again.
  delay(5000);
}


void WriteFile(const char* path, const char* message) {
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