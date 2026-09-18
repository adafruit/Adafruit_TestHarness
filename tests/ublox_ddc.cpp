// SPDX-License-Identifier: MIT
#include <assert.h>

#include <type_traits>

#include "../fixtures/ublox/UBloxDDCFixture.h"
#include "Adafruit_BusIO_Register.h"

int main() {
  using adafruit_test::UBloxDDCFixture;
  static_assert(!std::is_copy_constructible<UBloxDDCFixture>::value,
                "Fixture callbacks must not retain a different owner");
  UBloxDDCFixture bus;
  Adafruit_I2CDevice device(0x42, &bus);
  Adafruit_I2CDevice wrong(0x43, &bus);
  Adafruit_BusIO_Register count(&device, 0xFD, 2);
  Adafruit_BusIO_Register stream(&device, 0xFF, 1);
  uint8_t data[4] = {};
  assert(device.begin() && !wrong.begin());
  assert(count.read(data, 2) && data[0] == 0 && data[1] == 0);
  assert(!stream.read(data, 1));
  bus.input = {0xFF, 0, 42};
  assert(count.read(data, 2) && data[1] == 3 && bus.input.size() == 3);
  assert(!stream.read(data, 4) && bus.input.size() == 3);
  assert(stream.read(data, 2) && data[0] == 0xFF && data[1] == 0);
  bus.failRead = true;
  assert(!stream.read(data, 1) && bus.input.front() == 42);
  bus.failRead = false;
  assert(stream.read(data, 1) && data[0] == 42 && bus.input.empty());
  bus.reportedCount = 0x1234;
  assert(count.read(data, 2) && data[0] == 0x12 && data[1] == 0x34);
  assert(bus.countReads == 3);
  assert(!count.read(data, 1) && bus.countReads == 3);
  assert(!device.write(data, 1));
  assert(device.write(data, 2));
  bus.capacity = 2;
  assert(!device.write(data, 3));
  bus.failWrite = 1;
  assert(!device.write(data, 2) && bus.writes.size() == 1);
  UBloxDDCFixture otherBus(0x43);
  Adafruit_I2CDevice second(0x43, &otherBus);
  assert(second.begin() && otherBus.input.empty() && otherBus.countReads == 0);
}
