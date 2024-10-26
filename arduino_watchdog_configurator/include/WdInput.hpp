#ifndef WD_INPUT_MSG_HPP
#define WD_INPUT_MSG_HPP

#include <stdint.h>

using ulong = unsigned long;

class WdInputMsg {
private:
  const uint8_t mStart_bytes[2] = {kInputMsgStartByte1,
                                   kInputMsgStartByte2}; // Start bytes
  uint8_t mCmd;                                          // Command byte
  uint16_t mCrc16;                                       // CRC16 checksum

public:
  static constexpr uint8_t kInputMsgStartByte1 = 'W'; // Start byte 1
  static constexpr uint8_t kInputMsgStartByte2 = 'C'; // Start byte 2

  static constexpr uint8_t kDisableByte = 0x00; // Disable Byte
  static constexpr uint8_t kEnableByte = 0x01;  // Enable Byte
  static constexpr uint8_t kGetConfigurationByte =
      0x02; // GetConfiguration Byte

  enum class Command : uint8_t {
    Disable = kDisableByte,
    Enable = kEnableByte,
    GetConfiguration = kGetConfigurationByte,
    // Add more commands as needed
  };

  // Constructor
  WdInputMsg() : mCmd(static_cast<uint8_t>(Command::Disable)), mCrc16(0x0000) {}

  // Getter for mcmd
  uint8_t getCmd() const { return mCmd; }

  // Getter for mcrc16
  uint16_t getCRC16() const { return mCrc16; }

  // Setter for mcmd
  void setCmd(Command command) { mCmd = static_cast<uint8_t>(command); }
  void setCmd(uint8_t command) { mCmd = command; }

  // Setter for mcrc16
  void setCRC16(uint16_t crc) { mCrc16 = crc; }
};

#endif // WD_INPUT_MSG_HPP
