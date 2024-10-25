#ifndef WD_RESPONSE_HPP
#define WD_RESPONSE_HPP

#include <stdint.h>
// Ensure this header provides the calcCRC16 function
#include "CRC.h"

class WdAck {
  public:
    static constexpr uint8_t kAck = 0x01;  // Acknowledgment
    static constexpr uint8_t kNack = 0x00; // No acknowledgment
};

class WdStatus {
  public:
    static constexpr uint8_t kDisabled = 0x00;       // Watchdog is disabled
    static constexpr uint8_t kEnabled = 0x01;        // Watchdog is enabled
    static constexpr uint8_t kPinWriteError = 0x02;  // Failed to write to pin
    static constexpr uint8_t kInvalidCommand = 0x04; // Invalid command received
    static constexpr uint8_t kInvalidCrc = 0x08;     // CRC validation failed
    static constexpr uint8_t kTimeoutError = 0x10;   // Message timeout occurred
};

class WdResponse {

  public:
    static constexpr uint8_t kResponseStartByte1 = 'W'; // Response start byte 1
    static constexpr uint8_t kResponseStartByte2 = 'R'; // Response start byte 2

    static constexpr uint8_t kDefaultAck = WdAck::kNack;           // Default acknowledgment
    static constexpr uint8_t kDefaultStatus = WdStatus::kDisabled; // Default status

    uint8_t  mAck;    // Acknowledgment byte
    uint8_t  mStatus; // Status byte
    uint16_t mCrc16;  // CRC16 checksum for response

    // Calculate and set the CRC16 for the response message
    void setCrc16() {
        // uint8_t wDResponse[] = {mStartBytes[0], mStartBytes[1], mAck, mStatus};

        mCrc16 = calcCRC16(wDResponse, sizeof(wDResponse));
    }

    uint8_t *getRawMsg() const {
        fillRawResponseMsg();

        return mResponseRawMsg; // Return pointer to the raw message
    }

    // Constructor
    WdResponse() : mAck(WdAck::kNack), mStatus(WdStatus::kDisabled), mCrc16(0x0000) {}

  private:
    const uint8_t mStartBytes[2] = {kResponseStartByte1, kResponseStartByte2}; // Start bytes
    mutable uint8_t
        mResponseRawMsg[sizeof(mStartBytes) + sizeof(mAck) + sizeof(mStatus) + sizeof(mCrc16)];

    void fillRawResponseMsg() const {
        uint8_t index = 0;
        size_t  rawResponseMsgSize = sizeof(mResponseRawMsg);

        mResponseRawMsg[rawResponseMsgSize > index ? index++ : index] = mStartBytes[0];
        mResponseRawMsg[rawResponseMsgSize > index ? index++ : index] = mStartBytes[1];
        mResponseRawMsg[rawResponseMsgSize > index ? index++ : index] = mAck;
        mResponseRawMsg[rawResponseMsgSize > index ? index++ : index] = mStatus;

        mResponseRawMsg[rawResponseMsgSize > index ? index++ : index] =
            static_cast<uint8_t>(mCrc16 & 0xFF);

        mResponseRawMsg[rawResponseMsgSize > index ? index++ : index] =
            static_cast<uint8_t>((mCrc16 >> 8) & 0xFF);
    }
};

#endif // WD_RESPONSE_HPP
