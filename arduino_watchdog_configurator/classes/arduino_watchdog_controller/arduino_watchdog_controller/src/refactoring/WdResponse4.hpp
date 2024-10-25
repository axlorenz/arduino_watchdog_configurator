#ifndef WD_RESPONSE_HPP
#define WD_RESPONSE_HPP

#include "CRC.h"
#include <array>
#include <stdint.h>

template <size_t StatusSize = 1> class WdResponse {
  private:
    // Start bytes for the response
    static constexpr uint8_t kResponseStartByte1 = 'W'; // Response start byte 1
    static constexpr uint8_t kResponseStartByte2 = 'R'; // Response start byte 2

    // Size of the raw message
    static constexpr size_t kRawMsgSize =
        2 + 1 + StatusSize + 2; // Start bytes, Ack, Status bytes, CRC

    // Raw response message array
    mutable std::array<uint8_t, kRawMsgSize> mResponseRawMsg = {};

    // Acknowledgment byte
    uint8_t mAck;

    // Status bytes array
    using StatusArray = std::array<uint8_t, StatusSize>;
    StatusArray mStatus;

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
    // static constexpr uint8_t kDefaultAck = 0x00;

    // Status codes returned by the system
    enum class WdAck : uint8_t {
        Acknowledged = 0x06,    // Acknowledged
        NotAcknowledged = 0x15, // Not Acknowledged
    };

    // Status codes returned by the system
    enum class WdStatus : uint8_t {
        Disabled = 0x00,       // Watchdog is disabled
        Enabled = 0x01,        // Watchdog is enabled
        PinWriteError = 0x02,  // Failed to write to pin
        InvalidCommand = 0x04, // Invalid command received
        InvalidCrc = 0x08,     // CRC validation failed
        TimeoutError = 0x10    // Message timeout occurred
    };

    // Default constructor
    WdResponse() : mAck{static_cast<uint8_t>(WdAck::NotAcknowledged)}, mStatus{} {
        mStatus.fill(static_cast<uint8_t>(WdStatus::Disabled)); // Initialize all status bytes
    }

    // Method to get raw message array (with calculated CRC16)
    const std::array<uint8_t, kRawMsgSize> &getRawMsg() const {
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
