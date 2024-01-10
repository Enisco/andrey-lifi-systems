
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266Firebase.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "PinDefinitionsAndMore.h"

#define _SSID "Hello_"
#define _PASSWORD "987654321"
#define REFERENCE_URL "https://medilifi-a63d4-default-rtdb.firebaseio.com/"

#define DISABLE_CODE_FOR_RECEIVER
#include <IRremote.hpp>

#define DELAY_AFTER_SEND 500
#define DELAY_AFTER_LOOP 5000
// #define USE_DEFAULT_FEEDBACK_LED_PIN false

SoftwareSerial mySerial(D5, D7);  // RX, TX

unsigned long dataMillis = 0;
int count = 0;

Firebase firebase(REFERENCE_URL);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(_SSID);
  WiFi.begin(_SSID, _PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // Begin IrSender
  IrSender.begin(DISABLE_LED_FEEDBACK);
  Serial.println(F("Send IR signals at pin 6"));

  // set the data rate for the SoftwareSerial port
  mySerial.begin(115200);
  delay(2000);

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  digitalWrite(LED_BUILTIN, HIGH);
}

int intValue1 = 240;  // Replace these with your desired integer values
int intValue2 = 123;
int intValue3 = 205;
uint8_t convertedHexValue1, convertedHexValue2, convertedHexValue3;

uint8_t sAddress = 0x01;
uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

char hexValue1[4];
char hexValue2[4];
char hexValue3[4];

void loop() {
  // run over and over
  if (mySerial.available() > 0) {                  // Check if data is available to read
    String data = mySerial.readStringUntil('\n');  // Read the data until newline character '\n'

    Serial.print("Received data: ");
    Serial.println(data);

    // Split the string data

    int size = 0;
    String* dataList = splitData(data, size);

    Serial.print("Split Data:  ");
    for (int i = 0; i < size; i++) {
      Serial.print(dataList[i]);
      Serial.print("  ");
    }
    Serial.println();

    int temperatureC = 0, heartRate = 0, spO2 = 0;

    temperatureC = dataList[0].toInt();

    if (dataList[2].toInt() == 1) {
      heartRate = dataList[1].toInt();
    }
    if (dataList[4].toInt() == 1) {
      spO2 = dataList[3].toInt();
    }

    delete[] dataList;

    sprintf(hexValue1, "%03X", temperatureC);
    sprintf(hexValue2, "%03X", heartRate);
    sprintf(hexValue3, "%03X", spO2);

    // Convert HEX to uint8_t
    sscanf(hexValue1, "%hhx", &convertedHexValue1);
    sscanf(hexValue2, "%hhx", &convertedHexValue2);
    sscanf(hexValue3, "%hhx", &convertedHexValue3);

    Serial.println(F("Send Apple"));
    Serial.flush();

    // Send to the receiver in sequence
    IrSender.sendApple(0x01, convertedHexValue1, sRepeats);
    delay(DELAY_AFTER_SEND);
    IrSender.sendApple(0x02, convertedHexValue2, sRepeats);
    delay(DELAY_AFTER_SEND);
    IrSender.sendApple(0x03, convertedHexValue3, sRepeats);
    delay(DELAY_AFTER_SEND);

    // Send data to the realtime database.
    firebase.setInt("users/John Doe/vitals/tempC", temperatureC);
    firebase.setInt("users/John Doe/vitals/heartRate", heartRate);
    firebase.setInt("users/John Doe/vitals/spO2", spO2);

    sRepeats++;

    if (sRepeats > 4) {
      sRepeats = 4;
    }

    // delay(3000);
  }
}

String* splitData(String data, int& size) {
  int startIndex = data.indexOf(": ") + 2;            // Find the start index after ": "
  String remainingData = data.substring(startIndex);  // Extract data after ": "

  int count = 1;  // Count the number of elements in the list
  for (int i = 0; i < remainingData.length(); i++) {
    if (remainingData.charAt(i) == ',') {
      count++;
    }
  }

  size = count;                          // Update the size of the list
  String* dataList = new String[count];  // Create a dynamic array of Strings

  int index = 0;
  int commaIndex = 0;
  for (int i = 0; i < remainingData.length(); i++) {
    if (remainingData.charAt(i) == ',') {
      dataList[index++] = remainingData.substring(commaIndex, i);  // Extract substring till comma
      commaIndex = i + 1;                                          // Update comma index
    }
  }
  dataList[index] = remainingData.substring(commaIndex);  // Extract the last substring after the last comma

  return dataList;  // Return the array of Strings
}
