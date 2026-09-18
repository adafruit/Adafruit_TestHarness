// SPDX-License-Identifier: MIT
/** @file UBloxDDCFixture.h Optional u-blox DDC register model for host tests.
 */
#pragma once
#include <deque>

#include "Wire.h"

namespace adafruit_test {

/// Model the u-blox unread-byte count and data-stream registers only.
class UBloxDDCFixture : public TwoWire {
 public:
  std::deque<uint8_t> input; ///< Bytes queued by the simulated receiver.
  int reportedCount =
      -1; ///< Override the count register; -1 uses the queue size.
  size_t countReads = 0; ///< Number of successful count-register reads.

  /// Accept the configured device address and implement its two DDC registers.
  explicit UBloxDDCFixture(uint8_t deviceAddress = 0x42) {
    onBegin = [deviceAddress](uint8_t address) {
      return address == deviceAddress;
    };
    onWrite = [deviceAddress](uint8_t address, const uint8_t*, size_t size) {
      // u-blox requires at least two bytes in a raw data write.
      return address == deviceAddress && size >= 2;
    };
    onReadRegister = [this, deviceAddress](uint8_t address, uint8_t reg,
                                           uint8_t* data, size_t size) {
      if (address != deviceAddress)
        return false;
      if (reg == 0xFD && size == 2) {
        // The available-byte count is transmitted most-significant byte first.
        ++countReads;
        uint16_t count = input.size();
        if (reportedCount >= 0)
          count = reportedCount;
        data[0] = count / 256;
        data[1] = count;
        return true;
      }
      if (reg != 0xFF || input.size() < size)
        return false;
      for (size_t i = 0; i < size; ++i) {
        data[i] = input.front();
        input.pop_front();
      }
      return true;
    };
  }

  UBloxDDCFixture(const UBloxDDCFixture&) =
      delete; ///< Callbacks refer to this.
  UBloxDDCFixture& operator=(const UBloxDDCFixture&) =
      delete; ///< Not copyable.
};

} // namespace adafruit_test
