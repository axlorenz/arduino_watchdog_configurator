#include "CRC.h"
#include <Arduino.h>
#include <stdint.h>

const uint8_t       kPinNo = 7;
const unsigned long kThresholdMs = 30; // Timeout threshold in ms
const uint8_t       kWdDisableCmd = HIGH;
const uint8_t       kWdEnableCmd = LOW;
const uint8_t       kInputMsgStartByte1 = 'W';
const uint8_t       kInputMsgStartByte2 = 'C';

// Enumeration for acknowledgment
enum class WdAck : uint8_t { Ack = 0x06, Nack = 0x15 };

// Enumeration for commands
enum class WdCommand : uint8_t { Disable = 0x00, Enable = 0x01, GetConfiguration = 0x02 };

// Enumeration for status codes
enum class WdStatus : uint8_t {
    Disabled = 0x00,
    Enabled = 0x01,
    PinWriteError = 0x02,
    InvalidCommand = 0x04,
    InvalidCrc = 0x08,
    TimeoutError = 0x10
};

// Enumeration for input processing states
enum class InputProcessState {
    WaitForStartByte1,
    WaitForStartByte2,
    WaitForCmd,
    WaitForCrc1,
    WaitForCrc2
};

// Input message structure
class WdInputMsg {
  public:
    uint8_t  mStartBytes[2]; // Start bytes
    uint8_t  mCmd;           // Command byte
    uint16_t mCrc16;         // CRC16 checksum
};

// Response message structure
class WdResponse {
  public:
    const uint8_t mStartBytes[2] = {'W', 'R'};                        // Start bytes
    uint8_t       mAck = static_cast<uint8_t>(WdAck::Nack);           // Acknowledgment byte
    uint8_t       mStatus = static_cast<uint8_t>(WdStatus::Disabled); // Status byte
    uint16_t      mCrc16;                                             // CRC16 checksum

    void setCrc16() {
        mCrc16 = calcCRC16((uint8_t *)this, sizeof(WdResponse) - sizeof(m
