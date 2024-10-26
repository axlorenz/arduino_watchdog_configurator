#ifndef WD_MANAGER_HPP
#define WD_MANAGER_HPP

#include "WdController.hpp"
#include "WdInput.hpp"
#include "WdInputByteProcessor.hpp"
#include "WdResponse.hpp"
#include <stdint.h>

class WdManager {

private:
  WdInputMsg mWdInputMsg{};
  WdInputByteProcessor mWdInputByteProcessor;
  ulong currentTimeMs;

public:
  WdManager() : mWdInputMsg{}, mWdInputByteProcessor{mWdInputMsg} {}

  bool processMsgByte(uint8_t newByteIn) { currentTimeMs = millis(); }
};

#endif