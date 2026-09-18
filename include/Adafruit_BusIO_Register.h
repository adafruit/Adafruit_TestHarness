// SPDX-License-Identifier: MIT
/** @file Adafruit_BusIO_Register.h Host fake for explicit buffer register
 * reads. */
#pragma once
#include "Adafruit_I2CDevice.h"

/// Minimal 8-bit-address register fake; no scalar/bit-field or SPI operations.
class Adafruit_BusIO_Register {
 public:
  /// Bind a register; width is accepted for source compatibility only.
  Adafruit_BusIO_Register(Adafruit_I2CDevice* device, uint8_t address, uint8_t)
      : device(device), address(address) {}
  /// Read the explicitly requested byte count without endian conversion.
  bool read(uint8_t* data, size_t size) {
    return device && device->readRegister(address, data, size);
  }

 private:
  Adafruit_I2CDevice* device; ///< Caller-owned fake I2C device.
  uint8_t address; ///< Register address supplied to the peripheral callback.
};
