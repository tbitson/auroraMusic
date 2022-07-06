/**************************************************
  colorUtils.h - a collection of utilities for
  working with rgb24 colors.

  vers. 1.0  April 2021  tbitson

  colorWheel for Teensy with SmartMatrix V4
  adapted from Wheel by ladyAda @ adafruit.com

**************************************************/


#pragma once

#include <FastLED.h>


// prototypes
float randomf(float lower, float upper);
uint16_t XY(uint16_t x, uint16_t y);
void rgb24DimAll(uint8_t sf);
rgb24 rgb24SetColorBrightness(rgb24 color, uint8_t sf);
rgb24 hsv2rbg24(uint8_t h, uint8_t s, uint8_t v);
rgb24 wheel8(uint8_t);
rgb24 wheel8Sat(uint8_t, uint8_t);
void printColor(rgb24 color);



// led array buffer
extern rgb24* rgb24Buffer;





// define black, also used for off
rgb24 BLACK = {0, 0, 0};

// white is used for full-current test
rgb24 WHITE = {0xFF, 0xFF, 0xFF};

// for ease of use, define some common colors
rgb24 RED     = {0xFF, 0x00, 0x00};
rgb24 GREEN   = {0x00, 0xFF, 0x00};
rgb24 BLUE    = {0x00, 0x00, 0xFF};
rgb24 PURPLE  = {0xFF, 0x00, 0xFF};


// basic 7 colors plus black
rgb24 rgb24Colors8[8] = {
  // r     g    b
  {0x00, 0x00, 0xFF},   // 1 BLUE
  {0x00, 0x88, 0xFF},   // 2 LT BLUE
  {0x00, 0xFF, 0x00},   // 3 GREEN
  {0x77, 0xFF, 0x33},   // 4 LT GREEN
  {0xFF, 0xFF, 0x00},   // 5 YELLOW
  {0xFF, 0x88, 0x00},   // 6 ORANGE
  {0xFF, 0x00, 0x00},   // 7 RED
  {0x99, 0x00, 0x88},   // 8 VIOLET
};




// 16 colors table
rgb24 rgb24Colors16[16]  = {
  {0x11, 0x00, 0x77}, {0x00, 0x00, 0x88}, {0x00, 0x00, 0xff}, {0x00, 0x66, 0xff},
  {0x00, 0xaa, 0xff}, {0x00, 0xff, 0xff}, {0x00, 0xff, 0xaa}, {0x00, 0xff, 0x55},
  {0x00, 0xff, 0x00}, {0x88, 0xff, 0x00}, {0xbb, 0xff, 0x00}, {0xff, 0xff, 0x00},
  {0xff, 0xaa, 0x00}, {0xff, 0x55, 0x00}, {0xff, 0x00, 0x00}, {0xff, 0x00, 0x88}
};




// function to create a (kinda) random float
float randomf(float lower, float upper)
{
  // random only creates ints, so scale up
  int32_t iVal = random(lower * 10000, upper * 10000);
  // then scale back down
  float fVal = (float)iVal / 10000.0;

  return fVal;
}




// translates from x, y position into a
// linear index into the LED array
uint16_t XY(uint16_t x, uint16_t y)
{
#ifdef BOUNDS_CHECKING
  if (x > kScreenWidth - 1)
  {
    printValue("x out of bounds", x);
    x = 0;
    pattern = 0;
  }

  if (y > kScreenHeight - 1)
  {
    printValue("y out of bounds", y);
    y = 0;
    pattern = 0;
  }
#endif

  return (y * kScreenWidth) + x;
}



// scale pixel brightness, sf of 255 = 0 (off), sf of 0 = no scaling
rgb24 rgb24SetColorBrightness(rgb24 color, uint8_t sf)
{
  color.red   = scale8(color.red, sf);
  color.green = scale8(color.green, sf);
  color.blue  = scale8(color.blue, sf);
  return color;
}



// dim entire display
void rgb24DimAll(uint8_t sf)
{
  rgb24 color;

  while (backgroundLayer.isSwapPending())
    ;

  // get a pointer to the led array buffer
  rgb24Buffer = backgroundLayer.backBuffer();

  // apply dimming scale factor to each led using fastLed library
  for (uint32_t i = 0; i < kNumLEDs; i++)
  {
    color = rgb24Buffer[i];
    color.red = scale8(color.red, sf);
    color.green = scale8(color.green, sf);
    color.blue = scale8(color.blue, sf);
    rgb24Buffer[i] = color;
  }

  // update display
  backgroundLayer.swapBuffers();

  // get an updated pointer to the led array buffer
  rgb24Buffer = backgroundLayer.backBuffer();
}




// 8-bit color wheel - this version starts
// at blue (0) -> green (85) -> red (170) -> blue (255)
// to match the defined colors8 and colors16
rgb24 wheel8(uint8_t wheelPos)
{
  rgb24 color;

  // blue -> green
  if (wheelPos < 85)
  {
    color.red = 0;
    color.green =  wheelPos * 3;
    color.blue = 255 - (wheelPos * 3);
  }

  // green to red
  else if (wheelPos < 170)
  {
    wheelPos -= 85;

    color.red = wheelPos * 3;
    color.green = 255 - (wheelPos * 3);
    color.blue = 0;
  }

  // red to blue
  else
  {
    wheelPos -= 170;

    color.red = 255 - (wheelPos * 3);
    color.green =  0;
    color.blue = wheelPos * 3;
  }

  // printColor(color);

  return color;
}





// saturated 8-bit color wheel - experimental
rgb24 wheel8Sat(uint8_t wheelPos, uint8_t offset = 0)
{
  rgb24 color;

  wheelPos += offset;

  // 0 to 42 blue -> cyan (43 steps)
  if (wheelPos < 43)
  {
    color.red = 0;
    color.green =  wheelPos * 6;
    color.blue = 255;
  }

  // 43 to 85 cyan -> green (43 steps)
  else if (wheelPos < 86)
  {
    color.red = 0;
    color.green =  255;
    color.blue = 255 - ((wheelPos - 43) * 6);
  }

  // 86 to 127 green -> yellow (42 steps)
  else if (wheelPos < 128)
  {
    color.red = ((wheelPos - 86) * 6);
    color.green = 255;
    color.blue = 0;
  }

  // 128 to 170 yellow -> red (43 steps)
  else if (wheelPos < 171)
  {
    color.red = 255;
    color.green = 255 - ((wheelPos -  128) * 6);
    color.blue = 0;
  }

  // 171 to 212 red to violet (42 steps)
  else if (wheelPos < 213)
  {
    color.red = 255;
    color.green =  0;
    color.blue = (wheelPos - 171) * 6;
  }

  // 213 to 255  violet -> blue (43 steps)
  else
  {
    color.red = 255 - ((wheelPos - 213) * 6) ;
    color.green =  0;
    color.blue = 255;
  }

  // printColor(color);

  return color;
}



// dev tool to shown pixel color
void printColor(rgb24 color)
{
  printValue("color values:");
  printValue("color.red",   color.red);
  printValue("color.green", color.green);
  printValue("color.blue",  color.blue);
  Serial.println();
}



// full html color table - uncomment if needed
/*
  typedef enum {
  AliceBlue = 0xF0F8FF,
  Amethyst = 0x9966CC,
  AntiqueWhite = 0xFAEBD7,
  Aqua = 0x00FFFF,
  Aquamarine = 0x7FFFD4,
  Azure = 0xF0FFFF,
  Beige = 0xF5F5DC,
  Bisque = 0xFFE4C4,
  Black = 0x000000,
  BlanchedAlmond = 0xFFEBCD,
  Blue = 0x0000FF,
  BlueViolet = 0x8A2BE2,
  Brown = 0xA52A2A,
  BurlyWood = 0xDEB887,
  CadetBlue = 0x5F9EA0,
  Chartreuse = 0x7FFF00,
  Chocolate = 0xD2691E,
  Coral = 0xFF7F50,
  CornflowerBlue = 0x6495ED,
  Cornsilk = 0xFFF8DC,
  Crimson = 0xDC143C,
  Cyan = 0x00FFFF,
  DarkBlue = 0x00008B,
  DarkCyan = 0x008B8B,
  DarkGoldenrod = 0xB8860B,
  DarkGray = 0xA9A9A9,
  DarkGrey = 0xA9A9A9,
  DarkGreen = 0x006400,
  DarkKhaki = 0xBDB76B,
  DarkMagenta = 0x8B008B,
  DarkOliveGreen = 0x556B2F,
  DarkOrange = 0xFF8C00,
  DarkOrchid = 0x9932CC,
  DarkRed = 0x8B0000,
  DarkSalmon = 0xE9967A,
  DarkSeaGreen = 0x8FBC8F,
  DarkSlateBlue = 0x483D8B,
  DarkSlateGray = 0x2F4F4F,
  DarkSlateGrey = 0x2F4F4F,
  DarkTurquoise = 0x00CED1,
  DarkViolet = 0x9400D3,
  DeepPink = 0xFF1493,
  DeepSkyBlue = 0x00BFFF,
  DimGray = 0x696969,
  DimGrey = 0x696969,
  DodgerBlue = 0x1E90FF,
  FireBrick = 0xB22222,
  FloralWhite = 0xFFFAF0,
  ForestGreen = 0x228B22,
  Fuchsia = 0xFF00FF,
  Gainsboro = 0xDCDCDC,
  GhostWhite = 0xF8F8FF,
  Gold = 0xFFD700,
  Goldenrod = 0xDAA520,
  Gray = 0x808080,
  Grey = 0x808080,
  Green = 0x008000,
  GreenYellow = 0xADFF2F,
  Honeydew = 0xF0FFF0,
  HotPink = 0xFF69B4,
  IndianRed = 0xCD5C5C,
  Indigo = 0x4B0082,
  Ivory = 0xFFFFF0,
  Khaki = 0xF0E68C,
  Lavender = 0xE6E6FA,
  LavenderBlush = 0xFFF0F5,
  LawnGreen = 0x7CFC00,
  LemonChiffon = 0xFFFACD,
  LightBlue = 0xADD8E6,
  LightCoral = 0xF08080,
  LightCyan = 0xE0FFFF,
  LightGoldenrodYellow = 0xFAFAD2,
  LightGreen = 0x90EE90,
  LightGrey = 0xD3D3D3,
  LightPink = 0xFFB6C1,
  LightSalmon = 0xFFA07A,
  LightSeaGreen = 0x20B2AA,
  LightSkyBlue = 0x87CEFA,
  LightSlateGray = 0x778899,
  LightSlateGrey = 0x778899,
  LightSteelBlue = 0xB0C4DE,
  LightYellow = 0xFFFFE0,
  Lime = 0x00FF00,
  LimeGreen = 0x32CD32,
  Linen = 0xFAF0E6,
  Magenta = 0xFF00FF,
  Maroon = 0x800000,
  MediumAquamarine = 0x66CDAA,
  MediumBlue = 0x0000CD,
  MediumOrchid = 0xBA55D3,
  MediumPurple = 0x9370DB,
  MediumSeaGreen = 0x3CB371,
  MediumSlateBlue = 0x7B68EE,
  MediumSpringGreen = 0x00FA9A,
  MediumTurquoise = 0x48D1CC,
  MediumVioletRed = 0xC71585,
  MidnightBlue = 0x191970,
  MintCream = 0xF5FFFA,
  MistyRose = 0xFFE4E1,
  Moccasin = 0xFFE4B5,
  NavajoWhite = 0xFFDEAD,
  Navy = 0x000080,
  OldLace = 0xFDF5E6,
  Olive = 0x808000,
  OliveDrab = 0x6B8E23,
  Orange = 0xFFA500,
  OrangeRed = 0xFF4500,
  Orchid = 0xDA70D6,
  PaleGoldenrod = 0xEEE8AA,
  PaleGreen = 0x98FB98,
  PaleTurquoise = 0xAFEEEE,
  PaleVioletRed = 0xDB7093,
  PapayaWhip = 0xFFEFD5,
  PeachPuff = 0xFFDAB9,
  Peru = 0xCD853F,
  Pink = 0xFFC0CB,
  Plaid = 0xCC5533,
  Plum = 0xDDA0DD,
  PowderBlue = 0xB0E0E6,
  Purple = 0x800080,
  Red = 0xFF0000,
  RosyBrown = 0xBC8F8F,
  RoyalBlue = 0x4169E1,
  SaddleBrown = 0x8B4513,
  Salmon = 0xFA8072,
  SandyBrown = 0xF4A460,
  SeaGreen = 0x2E8B57,
  Seashell = 0xFFF5EE,
  Sienna = 0xA0522D,
  Silver = 0xC0C0C0,
  SkyBlue = 0x87CEEB,
  SlateBlue = 0x6A5ACD,
  SlateGray = 0x708090,
  SlateGrey = 0x708090,
  Snow = 0xFFFAFA,
  SpringGreen = 0x00FF7F,
  SteelBlue = 0x4682B4,
  Tan = 0xD2B48C,
  Teal = 0x008080,
  Thistle = 0xD8BFD8,
  Tomato = 0xFF6347,
  Turquoise = 0x40E0D0,
  Violet = 0xEE82EE,
  Wheat = 0xF5DEB3,
  White = 0xFFFFFF,
  WhiteSmoke = 0xF5F5F5,
  Yellow = 0xFFFF00,
  YellowGreen = 0x9ACD32,

  // LED RGB color that roughly approximates
  // the color of incandescent fairy lights,
  // assuming that you're using FastLED
  // color correction on your LEDs (recommended).
  FairyLight = 0xFFE42D,
  // If you are using no color correction, use this
  FairyLightNCC = 0xFF9D2A

  } HTMLColorCode;

*/
