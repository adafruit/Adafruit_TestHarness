// SPDX-License-Identifier: MIT
/** @file Adafruit_I2CDevice.h Minimal callback-driven host BusIO device fake.
 */
#pragma once
#include "Wire.h"

/// Small BusIO-compatible surface; peripheral behavior is supplied by
/// callbacks.
class Adafruit_I2CDevice {
 public:
  /// Bind an I2C address to a caller-owned fake bus.
  Adafruit_I2CDevice(uint8_t address, TwoWire* wire = &Wire)
      : wire(wire), address(address) {}
  /// Ask the fixture whether a device is present at this address.
  bool begin() {
    return wire && wire->onBegin && wire->onBegin(address);
  }
  /// Return the configured transaction capacity.
  size_t maxBufferSize() {
    return wire ? wire->capacity : 0;
  }
  /// Capture successful writes after the peripheral callback accepts them.
  bool write(const uint8_t* data, size_t size) {
    if (!wire || !data || !size || size > wire->capacity)
      return false;
    if (wire->failWrite >= 0 && (size_t)wire->failWrite == wire->writes.size())
      return false;
    if (!wire->onWrite || !wire->onWrite(address, data, size))
      return false;
    wire->writes.emplace_back(data, data + size);
    return true;
  }
  /// Internal hook used by the register fake, not a real BusIO public method.
  bool readRegister(uint8_t reg, uint8_t* data, size_t size) {
    if (!wire || !data || !size || size > wire->capacity || wire->failRead)
      return false;
    return wire->onReadRegister &&
           wire->onReadRegister(address, reg, data, size);
  }

 private:
  TwoWire* wire;   ///< Caller-owned fake bus.
  uint8_t address; ///< I2C device address passed to every callback.
};
