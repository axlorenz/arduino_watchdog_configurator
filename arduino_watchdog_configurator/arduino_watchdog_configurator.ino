// #include <Arduino.h>
#include "array/Array/Array.h"
#include "crc/CRC.h"

#include "include/WdInput.hpp"
#include "include/WdResponse.hpp"

void setup() {
  // Initialize the serial communication at 9600 baud rate
  Serial.begin(9600);

  // Print a hello message
  Serial.println("Hello, Arduino World!");
  uint8_t data[] = {0x44, 0x55};

  uint16_t sol = calcCRC16(&data[0], sizeof(data));

  Array<uint8_t, 5> testarray;

  WdInputMsg wdInputMsg{};
  WdResponse<1> wdResponse{};

  auto &rawResponseMsg = wdResponse.getRawMsg();
  Serial.write(rawResponseMsg.data(), sizeof(rawResponseMsg));
}

void loop() {
  // Continuously print a message every second
  Serial.println("Running...");

  // Wait for 1 second (1000 milliseconds)
  delay(1000);
}
