// SPDX-License-Identifier: MIT
/** @file UBloxTestHelpers.h Optional packet helpers for u-blox driver tests. */
// Optional helpers requiring the Adafruit_uBlox and Adafruit_GPS headers.
#pragma once
#include <Adafruit_TestHarness.h>
#include <assert.h>
#include <limits.h>

#include <functional>
#include <string>
#include <vector>

#include "Adafruit_UBX.h"
#include "Adafruit_UBloxDDC.h"
#include "UBloxDDCFixture.h"
using adafruit_test::UBloxDDCFixture;
using Bytes = std::vector<uint8_t>; ///< Host-side protocol byte buffer.
/// Build a UBX frame with its length and two-byte checksum.
inline Bytes packet(uint8_t cls, uint8_t id, const Bytes& payload = {}) {
  Bytes result = {0xB5,
                  0x62,
                  cls,
                  id,
                  (uint8_t)payload.size(),
                  (uint8_t)(payload.size() / 256)};
  result.insert(result.end(), payload.begin(), payload.end());
  uint8_t a = 0, b = 0;
  for (size_t i = 2; i < result.size(); ++i) {
    a += result[i];
    b += a;
  }
  result.push_back(a);
  result.push_back(b);
  return result;
}
/// Frame a supplied NMEA body using the shared GPS command builder.
inline Bytes sentence(const char* body) {
  char text[256];
  size_t count =
      Adafruit_NMEA::buildCommand(text, sizeof(text), body, strlen(body));
  assert(count);
  return Bytes(text, text + count);
}
using adafruit_test::FakeStream;
/// Dispatch all currently queued bytes through the receiver under test.
inline void drain(Adafruit_UBX& ubx, FakeStream& port) {
  while (port.available())
    ubx.checkMessages();
}
