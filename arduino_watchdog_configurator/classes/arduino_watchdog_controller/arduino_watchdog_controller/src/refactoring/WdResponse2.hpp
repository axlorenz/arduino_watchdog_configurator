#ifndef WD_RESPONSE_HPP
#define WD_RESPONSE_HPP

#include "CRC.h"
#include <array>
#include <stdint.h>

template <size_t StatusSize = 1> class WdResponse {
  public:
    static constexpr uint8_t kResponseStartByte1 = 'W'; // Response start byte 1
    static constexpr uint8_t kResponseStartByte2 = 'R'; // Response start byte 2

    static constexpr uint8_t kDefaultAck = 0x00;              // Default acknowledgment (NACK)
    static constexpr size_t kRawMsgSize = 4 + StatusSize + 2; // Start bytes, Ack, Status bytes, CRC

    static constexpr uint8_t kWdIsDisabledByte = 0x00;
    static constexpr uint8_t kWdIsEnabledByte = 0x01;
    static constexpr uint8_t kGetConfigurationByte = 0x02;

    static constexpr uint8_t kPinWriteErrorByte = 0x02;
    static constexpr uint8_t kInvalidCommandByte = 0x04;
    static constexpr uint8_t kInvalidCrcByte = 0x08;
    static constexpr uint8_t kTimeoutErrorByte = 0x10;

    using StatusArray = std::array<uint8_t, StatusSize>;

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
    uint8_t          mAck = kDefaultAck; // Acknowledgment byte
    StatusArray      mStatus = {};       // Status bytes array
    mutable uint16_t mCrc16 = 0x0000;    // CRC16 checksum

    // Default constructor
    WdResponse() : mAck(kDefaultAck), mCrc16(0x0000) {}

    // Method to get raw message array (with calculated CRC16)
    const std::array<uint8_t, kRawMsgSize> &getRawMsg() const {
        fillRawResponseMsg();

        return mResponseRawMsg;
    }

  private:
    // Start bytes for the response
    const std::array<uint8_t, 2> mStartBytes = {kResponseStartByte1, kResponseStartByte2};

    // Raw response message array
    mutable std::array<uint8_t, kRawMsgSize> mResponseRawMsg = {};

    // Fill the raw message array and calculate CRC16
    void fillRawResponseMsg() const {

        // Fill message array
        mResponseRawMsg[0] = mStartBytes[0];
        mResponseRawMsg[1] = mStartBytes[1];
        mResponseRawMsg[2] = mAck;

        // Copy status bytes
        for (size_t i = 0; i < StatusSize; ++i) {
            mResponseRawMsg[3 + i] = mStatus[i];
        }

        // Calculate CRC16 for the message up to the status bytes
        mCrc16 = calcCRC16(mResponseRawMsg.data(), 3 + StatusSize);

        // Set CRC16 bytes in the raw message
        mResponseRawMsg[3 + StatusSize] = static_cast<uint8_t>(mCrc16 & 0xFF);
        mResponseRawMsg[4 + StatusSize] = static_cast<uint8_t>((mCrc16 >> 8) & 0xFF);
    }
};

#endif // WD_RESPONSE_HPP
