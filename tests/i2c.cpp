// SPDX-License-Identifier: MIT
#include <assert.h>

#include "Adafruit_BusIO_Register.h"

int main() {
  TwoWire bus;
  Adafruit_I2CDevice device(0x20, &bus);
  Adafruit_I2CDevice other(0x21, &bus);
  Adafruit_BusIO_Register reg(&device, 0x10, 1);
  Adafruit_BusIO_Register otherReg(&other, 0x10, 1);
  uint8_t data[4] = {};
  assert(!device.begin());
  assert(!device.write(data, 1));
  assert(!reg.read(data, 1));

  bus.onBegin = [](uint8_t address) { return address == 0x20; };
  bus.onWrite = [](uint8_t address, const uint8_t* bytes, size_t size) {
    return address == 0x20 && size == 1 && bytes[0] == 42;
  };
  size_t reads = 0;
  bus.onReadRegister = [&reads](uint8_t address, uint8_t registerAddress,
                                uint8_t* bytes, size_t size) {
    ++reads;
    if (address != 0x20 || registerAddress != 0x10 || size != 2)
      return false;
    bytes[0] = 42;
    bytes[1] = 43;
    return true;
  };
  assert(device.begin() && !other.begin());
  assert(reg.read(data, 2) && data[0] == 42 && data[1] == 43);
  assert(!otherReg.read(data, 2));
  Adafruit_BusIO_Register unknown(&device, 0x11, 1);
  assert(!unknown.read(data, 2));
  assert(reads == 3);
  bus.failRead = true;
  assert(!reg.read(data, 2) && reads == 3);
  bus.failRead = false;
  bus.capacity = 1;
  assert(device.maxBufferSize() == 1 && !reg.read(data, 2));
  assert(reads == 3);
  assert(!reg.read(nullptr, 1) && !reg.read(data, 0));

  data[0] = 42;
  // A one-byte write is valid for this device; the core has no DDC minimum.
  assert(device.write(data, 1));
  assert(bus.writes == std::vector<std::vector<uint8_t>>({{42}}));
  assert(!other.write(data, 1));
  assert(!device.write(data, 2));
  assert(!device.write(nullptr, 1) && !device.write(data, 0));
  bus.failWrite = 1;
  assert(!device.write(data, 1) && bus.writes.size() == 1);
  bus.failWrite = -1;
  assert(device.write(data, 1) && bus.writes.size() == 2);

  TwoWire independent;
  Adafruit_I2CDevice missing(0x20, &independent);
  assert(!missing.begin() && independent.writes.empty());
  Adafruit_I2CDevice noBus(0x20, nullptr);
  Adafruit_BusIO_Register noDevice(nullptr, 0, 1);
  assert(!noBus.begin() && !noBus.write(data, 1));
  assert(!noBus.readRegister(0, data, 1) && noBus.maxBufferSize() == 0);
  assert(!noDevice.read(data, 1));
}
