/******************************************************************
  fastMath.h - fastLED math functions

  The fastLED library provide several fast integer math function.
  This file contains some function that use multiple fastLED math
  routines and interfaces.



********************************************************************/


#pragma once

#include <FastLED.h>



// BPM is 'beats per minute', or 'beats per 60000ms'.
// To avoid using the (slower) division operator, we
// want to convert 'beats per 60000ms' to 'beats per 65536ms',
// and then use a simple, fast bit-shift to divide by 65536.


// located in FastLED / lib8tion.h:
// beatsin8 generates an 8-bit sine wave at a given BPM, that oscillates within a given range.
// uint8_t beatsin8(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255,  uint32_t timebase = 0, uint8_t phase_offset = 0)
/// beatcos8 generates an 8-bit cos wave at a given BPM, that oscillates within a given range.

uint8_t beatcos8(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255, uint32_t timebase = 0, uint8_t phase_offset = 0)
{
  uint8_t beat = beat8(beats_per_minute, timebase);
  uint8_t beatcos = cos8(beat + phase_offset);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}



uint8_t beattriwave8(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255, uint32_t timebase = 0, uint8_t phase_offset = 0)
{
  uint8_t beat = beat8(beats_per_minute, timebase);
  uint8_t beatcos = triwave8(beat + phase_offset);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}


// generates a 8-bit sin of angle theta scaled from lowest to highest (default = 0 - 255)
uint8_t mapsin8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255)
{
  uint8_t beatsin = sin8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}


// generates a 8-bit cos of angle theta (0 - 255) scaled from lowest to highest
uint8_t mapcos8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255)
{
  uint8_t beatcos = cos8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}
