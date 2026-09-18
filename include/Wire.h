// SPDX-License-Identifier: MIT
/** @file Wire.h Callback-driven I2C state for the host BusIO fakes. */
#pragma once
#include <functional>
#include <vector>

#include "Arduino.h"

/// Minimal fake bus state, not an implementation of Arduino's TwoWire API.
struct TwoWire {
  size_t capacity = 32; ///< Maximum bytes per buffer operation.
  int failWrite = -1; ///< Fail after this many successful writes; -1 disables.
  bool failRead = false; ///< Fail reads before invoking the device callback.
  std::vector<std::vector<uint8_t>> writes; ///< Successful raw write buffers.

  /// Device detection callback, receiving the I2C address.
  std::function<bool(uint8_t)> onBegin;
  /// Write callback: I2C address, data, and byte count.
  std::function<bool(uint8_t, const uint8_t*, size_t)> onWrite;
  /// Register-read callback: I2C address, register, output buffer, byte count.
  std::function<bool(uint8_t, uint8_t, uint8_t*, size_t)> onReadRegister;
};

extern TwoWire Wire; ///< Default bus; unconfigured operations fail.
