#ifndef WD_INPUT_BYTE_PROCESSOR_HPP
#define WD_INPUT_BYTE_PROCESSOR_HPP

#include "WdInput.hpp"
#include <stdint.h>

class WdInputByteProcessor {

public:
  WdInputByteProcessor(WdInputMsg &wdInputMsg)
      : mLastReceivedTime{0}, mWdInputMsg{wdInputMsg},
        mCurrentState{WdInputProcessState::WaitForStartByte1} {}

  WdInputByteProcessor() = delete;

  enum class WdInputProcessState {
    WaitForStartByte1,
    WaitForStartByte2,
    WaitForCmd,
    WaitForCrc1,
    WaitForCrc2
  };

  enum class WdInputMessageProcessState : uint8_t {
    InputMessageIncomplete = 0,
    InputMessageComplete = 1
  };

  WdInputMessageProcessState processByte(const uint8_t newByteIn,
                                         ulong currentTimeMs) {

    WdInputMessageProcessState retInputMsgProcessState =
        WdInputMessageProcessState::InputMessageIncomplete;

    // Check for timeout in receiving data
    if ((mCurrentState != WdInputProcessState::WaitForStartByte1) &&
        (currentTimeMs - mLastReceivedTime > kMsgTimeoutThresholdMs)) {
      resetStateMachine();
      return retInputMsgProcessState;
    }

    // State machine to process the input message byte by byte
    switch (mCurrentState) {
    case WdInputProcessState::WaitForStartByte1:
      if (newByteIn == mWdInputMsg.getStartByte1()) {
        mCurrentState = WdInputProcessState::WaitForStartByte2;
      }
      break;

    case WdInputProcessState::WaitForStartByte2:
      if (newByteIn == mWdInputMsg.getStartByte2()) {
        mCurrentState = WdInputProcessState::WaitForCmd;
      } else {
        resetStateMachine();
      }
      break;

    case WdInputProcessState::WaitForCmd:
      mWdInputMsg.setCmd(newByteIn);
      mCurrentState = WdInputProcessState::WaitForCrc1;
      break;

    case WdInputProcessState::WaitForCrc1:
      mWdInputMsg.setCrc16Msb(newByteIn); // Store MSB of CRC
      mCurrentState = WdInputProcessState::WaitForCrc2;
      break;

    case WdInputProcessState::WaitForCrc2:
      // Store LSB of CRC
      mWdInputMsg.setCrc16Lsb(newByteIn); // Store MSB of CRC
      // Process the complete input message
      retInputMsgProcessState =
          WdInputMessageProcessState::InputMessageComplete;
      resetStateMachine();
      break;
    }

    return retInputMsgProcessState;
  }

private:
  static constexpr ulong kMsgTimeoutThresholdMs = 30; // Timeout threshold in ms
  ulong mLastReceivedTime; // Timestamp of last received byte
  WdInputMsg &mWdInputMsg;

  WdInputProcessState mCurrentState;

  // Reset the state machine and input message
  void resetStateMachine() {
    mCurrentState = WdInputProcessState::WaitForStartByte1;
    // Reset the input message
  }
};

#endif