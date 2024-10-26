#ifndef WD_INPUT_BYTE_PROCESSOR_HPP
#define WD_INPUT_BYTE_PROCESSOR_HPP

#include "WdInput.hpp"
#include <stdint.h>

class WdInputByteProcessor {
private:
  static constexpr ulong kMsgTimeoutThresholdMs = 30; // Timeout threshold in ms
  ulong lastReceivedTime; // Timestamp of last received byte
  WdInputMsg &mRWdInputMsg;

public:
  WdInputByteProcessor(WdInputMsg &wdInputMsg)
      : lastReceivedTime{0}, mRWdInputMsg{wdInputMsg} {}
};

#endif