#include "CRC.h"

const uint8_t kPinNo = 7;
const uint8_t kWdDisableCmd = HIGH;
const uint8_t kWdEnableCmd = LOW;

const uint8_t kResponseStartByte1 = 'W';
const uint8_t kResponseStartByte2 = 'R';

const uint8_t kInputMsgStartByte1 = 'W';
const uint8_t kInputMsgStartByte2 = 'C';

const uint8_t kAckByte = 0x06;
const uint8_t kNackByte = 0x15;

const uint8_t kDisableByte = 0x00;
const uint8_t kEnableByte = 0x01;
const uint8_t kGetConfigurationByte = 0x02;

const uint8_t kPinWriteErrorByte = 0x02;
const uint8_t kInvalidCommandByte = 0x04;
const uint8_t kInvalidCrcByte = 0x08;
const uint8_t kTimeoutErrorByte = 0x10;

enum class WdAck : uint8_t { Ack = kAckByte, Nack = kNackByte };

const unsigned long kThresholdMs = 30;    // Timeout threshold in ms
unsigned long       lastReceivedTime = 0; // Timestamp of last received byte

#define BAUDRATE 115200

// Commands for controlling the watchdog
enum class WdCommand : uint8_t {
    Disable = kDisableByte,                  // Disable watchdog
    Enable = kEnableByte,                    // Enable watchdog
    GetConfiguration = kGetConfigurationByte // Get watchdog status
};

// Status codes returned by the system
enum class WdStatus : uint8_t {
    Disabled = kDisableByte,              // Watchdog is disabled
    Enabled = kEnableByte,                // Watchdog is enabled
    PinWriteError = kPinWriteErrorByte,   // Failed to write to pin
    InvalidCommand = kInvalidCommandByte, // Invalid command received
    InvalidCrc = kInvalidCrcByte,         // CRC validation failed
    TimeoutError = kTimeoutErrorByte      // Message timeout occurred
};

// Input message format for commands
struct WdInputMsg {
    uint8_t  mStart_bytes[2]; // Start bytes (e.g., 'W' and 'C')
    uint8_t  mCmd;            // Command byte
    uint16_t mCrc16;          // CRC16 checksum
};

// Response message format to acknowledge or reject commands
struct WdResponse {
    const uint8_t mStartBytes[2] = {kResponseStartByte1, kResponseStartByte2};
    // Start bytes ('W' and 'R')
    uint8_t mAck = static_cast<uint8_t>(WdAck::Nack);
    // Acknowledgment byte
    uint8_t mStatus = static_cast<uint8_t>(WdStatus::Disabled);
    // Status byte
    uint16_t mCrc16;
    // CRC16 checksum for response

    // Calculate and set the CRC16 for the response message
    void setCrc16() {
        this->crc16 = calcCRC16((uint8_t *) this, sizeof(WdResponse) - sizeof(crc16));
    }
};

// States for processing the input command message
enum class InputProcessState {
    WaitForStartByte1,
    WaitForStartByte2,
    WaitForCmd,
    WaitForCrc1,
    WaitForCrc2
};

InputProcessState currentState = InputProcessState::WaitForStartByte1;
WdInputMsg        inputMsg;

// Configure the watchdog based on the command
void configureWatchdog(WdCommand cmd) {
    switch (cmd) {
        case WdCommand::Disable:
            digitalWrite(kPinNo, kWdDisableCmd);
            break;
        case WdCommand::Enable:
            digitalWrite(kPinNo, kWdEnableCmd);
            break;
        default:
            break;
    }
}

// Get the current watchdog configuration (enabled or disabled)
WdStatus getWatchdogConfiguration() {
    int pinState = digitalRead(kPinNo);

    if (pinState == kWdEnableCmd) {
        return WdStatus::Enabled;
    } else {
        return WdStatus::Disabled;
    }
}

// Process the input message and send an appropriate response
void processInputMsg() {
    WdResponse response;

    // Calculate CRC16 for the input message (excluding the CRC bytes)
    uint16_t calculatedCrc16 = calcCRC16((uint8_t *) &inputMsg.start_bytes,
                                         sizeof(inputMsg.start_bytes) + sizeof(inputMsg.cmd));

    // Validate CRC
    if (inputMsg.crc16 == calculatedCrc16) {
        // Process the command
        switch (static_cast<WdCommand>(inputMsg.cmd)) {
            case WdCommand::Disable:
                configureWatchdog(WdCommand::Disable);
                if (getWatchdogConfiguration() != WdStatus::Disabled) {
                    response.ack = static_cast<uint8_t>(WdAck::Nack);
                    response.status = static_cast<uint8_t>(WdStatus::PinWriteError);
                } else {
                    response.ack = static_cast<uint8_t>(WdAck::Ack);
                    response.status = static_cast<uint8_t>(WdStatus::Disabled);
                }
                break;

            case WdCommand::Enable:
                configureWatchdog(WdCommand::Enable);
                if (getWatchdogConfiguration() != WdStatus::Enabled) {
                    response.ack = static_cast<uint8_t>(WdAck::Nack);
                    response.status = static_cast<uint8_t>(WdStatus::PinWriteError);
                } else {
                    response.ack = static_cast<uint8_t>(WdAck::Ack);
                    response.status = static_cast<uint8_t>(WdStatus::Enabled);
                }
                break;

            case WdCommand::GetConfiguration:
                response.ack = static_cast<uint8_t>(WdAck::Ack);
                response.status = static_cast<uint8_t>(getWatchdogConfiguration());
                break;

            default:
                response.ack = static_cast<uint8_t>(WdAck::Nack);
                response.status = static_cast<uint8_t>(WdStatus::InvalidCommand);
                break;
        }
    } else {
        // Invalid CRC
        response.ack = static_cast<uint8_t>(WdAck::Nack);
        response.status = static_cast<uint8_t>(WdStatus::InvalidCrc);
    }

    // Set CRC for the response and send it
    response.setCrc16();
    Serial.write((uint8_t *) &response, sizeof(response));
}

// Reset the state machine and input message
void resetStateMachine() {
    currentState = InputProcessState::WaitForStartByte1;
    inputMsg = {}; // Reset the input message
}

// Setup function for initializing the system
void setup() {
    pinMode(kPinNo, OUTPUT);               // Set the pin as output
    configureWatchdog(WdCommand::Disable); // Disable watchdog initially
    Serial.begin(BAUDRATE);                // Start serial communication
}

// Main loop for receiving and processing serial data
void loop() {
    // Check if data is available from the serial port
    if (Serial.available() > 0) {
        unsigned long currentMillis = millis();
        lastReceivedTime = currentMillis; // Update last received time

        uint8_t receivedByte = Serial.read();

        // State machine to process the input message byte by byte
        switch (currentState) {
            case InputProcessState::WaitForStartByte1:
                if (receivedByte == kInputMsgStartByte1) {
                    inputMsg.start_bytes[0] = receivedByte;
                    currentState = InputProcessState::WaitForStartByte2;
                }
                break;

            case InputProcessState::WaitForStartByte2:
                if (receivedByte == kInputMsgStartByte2) {
                    inputMsg.start_bytes[1] = receivedByte;
                    currentState = InputProcessState::WaitForCmd;
                } else {
                    resetStateMachine();
                }
                break;

            case InputProcessState::WaitForCmd:
                inputMsg.cmd = receivedByte;
                currentState = InputProcessState::WaitForCrc1;
                break;

            case InputProcessState::WaitForCrc1:
                inputMsg.crc16 = (receivedByte << 8); // Store MSB of CRC
                currentState = InputProcessState::WaitForCrc2;
                break;

            case InputProcessState::WaitForCrc2:
                inputMsg.crc16 |= receivedByte; // Store LSB of CRC
                processInputMsg();              // Process the
                complete input message resetStateMachine();
                break;
        }
    }

    // Check for timeout in receiving data
    if (millis() - lastReceivedTime > kThresholdMs) {
        resetStateMachine(); // Reset if no data is received within
        the threshold

            // Send timeout error response
            WdResponse timeoutResponse;
        timeoutResponse.ack = static_cast<uint8_t>(WdAck::Nack);
        timeoutResponse.status = static_cast<uint8_t>(WdStatus::TimeoutError);
        timeoutResponse.setCrc16();

        Serial.write((uint8_t *) &timeoutResponse, sizeof(timeoutResponse));
    }
}
