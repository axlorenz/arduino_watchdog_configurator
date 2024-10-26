#ifndef WD_RESPONSE_HPP
#define WD_RESPONSE_HPP

#include "../array/Array/Array.h"
#include "../crc/CRC.h"
#include <stdint.h>

template <size_t StatusSize = 1> class WdResponse {
private:
  // Start bytes for the response
  static constexpr uint8_t kResponseStartByte1 = 'W'; // Response start byte 1
  static constexpr uint8_t kResponseStartByte2 = 'R'; // Response start byte 2

  static constexpr uint8_t kDisabledByte = 0x00;      // Watchdog is disabled
  static constexpr uint8_t kEnabledByte = 0x01;       // Watchdog is enabled
  static constexpr uint8_t kPinWriteErrorByte = 0x02; // Failed to write to pin
  static constexpr uint8_t kInvalidCommandByte =
      0x04;                                          // Invalid command received
  static constexpr uint8_t kInvalidCrcByte = 0x08;   // CRC validation failed
  static constexpr uint8_t kTimeoutErrorByte = 0x10; // Message timeout occurred

  // Size of the raw message
  static constexpr size_t kRawMsgSize =
      sizeof(kResponseStartByte1) + sizeof(kResponseStartByte2) + 1 +
      StatusSize + 2; // Start bytes, Ack, Status bytes, CRC

  using RawResponseArray = Array<uint8_t, kRawMsgSize>;

  // Raw response message array
  mutable RawResponseArray mResponseRawMsg = {};

  // Fill the raw message array and calculate CRC16
  void fillRawResponseMsg() const {
    uint8_t index = 0;

    // Fill message array
    mResponseRawMsg[index++] = kResponseStartByte1;
    mResponseRawMsg[index++] = kResponseStartByte2;
    mResponseRawMsg[index++] = mAck;

    // Copy status bytes
    for (size_t i = 0; i < StatusSize; ++i) {
      mResponseRawMsg[index++] = mStatus[i];
    }

    // Calculate CRC16 for the message up to the status bytes
    uint16_t crc16 = calcCRC16(mResponseRawMsg.data(), kRawMsgSize - 2);

    // Set CRC16 bytes in the raw message
    mResponseRawMsg[index++] = static_cast<uint8_t>((crc16 >> 8) & 0x00FF);
    mResponseRawMsg[index++] = static_cast<uint8_t>(crc16 & 0x00FF);
  }

public:
  // Default acknowledgment (NACK)
  static constexpr uint8_t kDefaultAck = 0x00;

  // Status codes returned by the system
  enum class WdAck : uint8_t {
    Acknowledged = 0x06,    // Acknowledged
    NotAcknowledged = 0x15, // Not Acknowledged
  };

  // Status codes returned by the system
  enum class WdStatus : uint8_t {
    Disabled = kDisabledByte,             // Watchdog is disabled
    Enabled = kEnabledByte,               // Watchdog is enabled
    PinWriteError = kPinWriteErrorByte,   // Failed to write to pin
    InvalidCommand = kInvalidCommandByte, // Invalid command received
    InvalidCrc = kInvalidCrcByte,         // CRC validation failed
    TimeoutError = kTimeoutErrorByte      // Message timeout occurred
  };

  // Public members
  uint8_t mAck;                                   // Acknowledgment byte
  using StatusArray = Array<uint8_t, StatusSize>; // Status bytes array
  StatusArray mStatus;                            // Status bytes array

  // Default constructor
  WdResponse() : mAck{static_cast<uint8_t>(WdAck::NotAcknowledged)} {
    mStatus.fill(static_cast<uint8_t>(WdStatus::Disabled));
  }

  // Method to get raw message array (with calculated CRC16)
  const RawResponseArray &getRawMsg() const {
    fillRawResponseMsg();
    return mResponseRawMsg;
  }

  bool setWdAck(WdAck wDAckIn) {
    mAck = static_cast<uint8_t>(wDAckIn);
    return true; // Always successful
  }

  bool setWdStatus(WdStatus wDStatusIn, uint8_t index = 0) {
    if (index < StatusSize) {
      mStatus[index] = static_cast<uint8_t>(wDStatusIn);
      return true; // Successful
    }
    return false; // Index out of range
  }
};

#endif // WD_RESPONSE_HPP
