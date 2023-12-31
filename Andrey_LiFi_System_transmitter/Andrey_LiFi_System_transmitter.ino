
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "PinDefinitionsAndMore.h"

#define DISABLE_CODE_FOR_RECEIVER
#include <IRremote.hpp>

#define DELAY_AFTER_SEND 500
#define DELAY_AFTER_LOOP 5000
#define USE_DEFAULT_FEEDBACK_LED_PIN false

SoftwareSerial mySerial(D5, D7);  // RX, TX

void setup() {
  Serial.begin(115200);

  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // Begin IrSender
  IrSender.begin(DISABLE_LED_FEEDBACK);
  Serial.println(F("Send IR signals at pin 6"));

  // set the data rate for the SoftwareSerial port
  mySerial.begin(115200);
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
