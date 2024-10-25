#ifndef WD_RESPONSE_HPP
#define WD_RESPONSE_HPP

#include "CRC.h"
#include <array>
#include <stdint.h>

template <size_t StatusSize = 1> class WdResponse {
  private:
    // Start bytes for the response
    const std::array<uint8_t, 2> mStartBytes = {kResponseStartByte1, kResponseStartByte2};

    // Fill the raw message array and calculate CRC16
    void fillRawResponseMsg() const {

        uint8_t index = 0;

        // Fill message array
        mResponseRawMsg[index++] = mStartBytes[0];
        mResponseRawMsg[index++] = mStartBytes[1];
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
    static constexpr uint8_t kResponseStartByte1 = 'W'; // Response start byte 1
    static constexpr uint8_t kResponseStartByte2 = 'R'; // Response start byte 2

    // static constexpr uint8_t kDefaultAck = 0x00;              // Default acknowledgment (NACK)
    static constexpr size_t kRawMsgSize =
        sizeof(mStartBytes) + 1 + StatusSize + 2; // Start bytes, Ack, Status bytes, CRC

    static constexpr uint8_t kAckByte = 0x06;
    static constexpr uint8_t kNackByte = 0x15;

    static constexpr uint8_t kWdIsDisabledByte = 0x00;
    static constexpr uint8_t kWdIsEnabledByte = 0x01;

    static constexpr uint8_t kPinWriteErrorByte = 0x02;
    static constexpr uint8_t kInvalidCommandByte = 0x04;
    static constexpr uint8_t kInvalidCrcByte = 0x08;
    static constexpr uint8_t kTimeoutErrorByte = 0x10;

    static constexpr uint16_t kDefaultCRC16 = 0x0000;

    using StatusArray = std::array<uint8_t, StatusSize>;

    // Status codes returned by the system
    enum class WdAck : uint8_t {
        Acknowledged = kAckByte,     // Acknowledged
        NotAcknowledged = kNackByte, // Not Acknowledged
    };

    // Status codes returned by the system
    enum class WdStatus : uint8_t {
        Disabled = kWdIsDisabledByte,         // Watchdog is disabled
        Enabled = kWdIsEnabledByte,           // Watchdog is enabled
        PinWriteError = kPinWriteErrorByte,   // Failed to write to pin
        InvalidCommand = kInvalidCommandByte, // Invalid command received
        InvalidCrc = kInvalidCrcByte,         // CRC validation failed
        TimeoutError = kTimeoutErrorByte      // Message timeout occurred
    };

    // Public members
    uint8_t     mAck;    // Acknowledgment byte
    StatusArray mStatus; // Status bytes array

    // Raw response message array
    mutable std::array<uint8_t, kRawMsgSize> mResponseRawMsg = {};

    // Default constructor
    WdResponse()
        : mAck{WdAck::NotAcknowledged}, mStatus{WdStatus::Disabled}, mCrc16{kDefaultCRC16} {}

    // Method to get raw message array (with calculated CRC16)
    const std::array<uint8_t, kRawMsgSize> &getRawMsg() const {
        fillRawResponseMsg();

        return mResponseRawMsg;
    }
};

#endif // WD_RESPONSE_HPP
