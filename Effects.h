
/****************************************************************************************
   Effect.h - Effects class

   Based on code by Jason Coon.

   The original version of this was written for a 32x32 pixel matrix. With the
   introduction of Teensy 3.5, 3.6, and 4.x, much larger displays are possible.
   In an attempt to make this campatible, the functions have been modified to work
   on larger arrays. The class functions that wern't required for audio patterns
   have been removed to minimize code space.

   last modified: 18Apr2021

**************************************************************************************/

#pragma once


#include "fastMath.h"
#include "colorUtils.h"


// dev use - enables some debugging
//#define EFFECTS_DEBUG


// led array & buffer values
extern rgb24* rgb24Buffer;



// non-class prototypes
void FillNoiseCentral(uint8_t scale);
CRGB HsvToRgb(uint8_t h, uint8_t s, uint8_t v);



CRGB HsvToRgb(uint8_t h, uint8_t s, uint8_t v)
{
  CHSV hsv = CHSV(h, s, v);
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);
  return rgb;
}



//---------------------------------------------------------------------


class Effects {

  public:
    rgb24 ledArray2[kNumLEDs];

    uint8_t noise[kMatrixWidth][kMatrixHeight];
    uint16_t noiseX = 0;
    uint16_t noiseY = 0;
    uint16_t noiseZ = 0;

    uint16_t noise_x;
    uint16_t noise_y;
    uint16_t noise_z;

    uint16_t noise_scale_x;
    uint16_t noise_scale_y;
    uint8_t noisesmoothing;

    uint8_t osci[6]; // the oscillators: linear ramps 0-255
    uint8_t p[6];// sin8(osci) swinging between 0 to kScreenWidth - 1




    void setup()
    {
      // init noise generator
      NoiseVariablesSetup();
      MoveOscillators();
    }


    void updateBuffer()
    {
      rgb24Buffer = backgroundLayer.backBuffer();
    }


    void NoiseVariablesSetup()
    {
      noisesmoothing = 200;

      noise_x = random16();
      noise_y = random16();
      noise_z = random16();
      noise_scale_x = 6000;
      noise_scale_y = 6000;
    }


    // set the speeds (and by that ratios) of the oscillators here
    void MoveOscillators()
    {
      osci[0] = osci[0] + 5;
      osci[1] = osci[1] + 2;
      osci[2] = osci[2] + 3;
      osci[3] = osci[3] + 4;
      osci[4] = osci[4] + 1;

      if (osci[4] % 2 == 0)
        osci[5] = osci[5] + 1;

      for (int i = 0; i < 4; i++)
      {
        // keep the result in the range of 0-kScreenWidth (matrix size)
        p[i] = map8(sin8(osci[i]), 0, kScreenWidth - 1);
      }
    }


    void FillNoise()
    {
      for (uint16_t i = 0; i < kScreenWidth; i++)
      {
        uint32_t ioffset = noise_scale_x * (i / 2 - kMatrixCenterY);
        for (uint16_t j = 0; j < kScreenHeight; j++)
        {
          uint32_t joffset = noise_scale_y * (j - kMatrixCenterY);
          uint8_t data = inoise16(noise_x + ioffset, noise_y + joffset, noise_z) >> 8;
          uint8_t olddata = noise[i][j];
          uint8_t newdata = scale8(olddata, noisesmoothing) + scale8(data, 256 - noisesmoothing);
          data = newdata;

          noise[i][j] = data;
        }
      }
    }


    // calculate noise matrix x and y define the center
    void FillNoiseCentral(uint8_t scale)
    {
      for (uint16_t i = 0; i < kScreenWidth; i++)
      {
        int ioffset = scale * (i - 8);
        for (uint16_t j = 0; j < kScreenHeight; j++)
        {
          uint16_t joffset = scale * (j - 8);
          noise[i][j] = inoise8(noiseX + ioffset, noiseY + joffset, noiseZ);
        }
      }
    }





    // give it a linear tail downwards
    void StreamDown(uint8_t scale)
    {
      for (uint16_t x = 0; x < kScreenWidth; x++)
      {
        for (uint16_t y = 1; y < kScreenHeight; y++)
        {
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x, y - 1)];
          rgb24Buffer[XY(x, y)].nscale8(scale);
        }
      }

      for (uint16_t x = 0; x < kScreenWidth; x++)
        rgb24Buffer[XY(x, kScreenHeight - 1)].nscale8(scale);
    }




    // give it a linear tail upwards
    void StreamUp(uint8_t scale)
    {
      for (uint16_t x = 0; x < kScreenWidth; x++)
      {
        for (uint16_t y = kScreenHeight - 2; y > 0; y--)
        {
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x, y + 1)];
          rgb24Buffer[XY(x, y)].nscale8(scale);
        }
      }

      for (uint16_t x = 0; x < kScreenWidth; x++)
        rgb24Buffer[XY(x, kScreenHeight - 1)].nscale8(scale);
    }




    // give it a linear tail to the right
    void StreamRight(uint8_t scale, uint16_t fromX = 0, uint16_t toX = kScreenWidth, uint16_t fromY = 0, uint16_t toY = kScreenHeight)
    {
      for (uint16_t x = fromX + 1; x < toX; x++)
      {
        for (uint16_t y = fromY; y < toY; y++)
        {
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x - 1, y)];
          rgb24Buffer[XY(x, y)].nscale8(scale);
        }
      }

      for (uint16_t y = fromY; y < toY; y++)
        rgb24Buffer[XY(0, y)].nscale8(scale);
    }



    // give it a linear tail to the left
    void StreamLeft(uint8_t scale, uint16_t fromX = kScreenWidth, uint16_t toX = 0, uint16_t fromY = 0, uint16_t toY = kScreenHeight)
    {
      for (uint16_t x = toX; x < fromX - 1; x++)
      {
        for (uint16_t y = fromY; y < toY; y++)
        {
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x + 1, y)];
          rgb24Buffer[XY(x, y)].nscale8(scale);
        }
      }
      for (uint16_t y = fromY; y < toY; y++)
        rgb24Buffer[XY(0, y)].nscale8(scale);
    }



    // give it a linear tail up and to the left
    void StreamUpAndLeft(uint8_t scale)
    {
      for (uint16_t x = 0; x < kScreenWidth - 1; x++)
      {
        for (uint16_t y = kScreenHeight - 2; y > 0; y--)
        {
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x + 1, y + 1)];
          rgb24Buffer[XY(x, y)].nscale8(scale);
        }
      }
      for (uint16_t x = 1; x < kScreenWidth; x++)
        rgb24Buffer[XY(x, kScreenHeight - 1)].nscale8(scale);

      for (uint16_t y = 0; y < kScreenHeight; y++)
        rgb24Buffer[XY(kScreenWidth - 1, y)].nscale8(scale);
    }



    // give it a linear tail up and to the right
    void StreamUpAndRight(uint8_t scale)
    {
      for (uint16_t x = 0; x < kScreenWidth - 1; x++)
      {
        for (uint16_t y = kScreenHeight - 2; y > 0; y--)
        {
          rgb24Buffer[XY(x + 1, y)] += rgb24Buffer[XY(x, y + 1)];
          rgb24Buffer[XY(x, y)].nscale8(scale);
        }
      }

      // fade the bottom row
      for (uint16_t x = 0; x < kScreenWidth; x++)
        rgb24Buffer[XY(x, kScreenHeight - 1)].nscale8(scale);

      // fade the right column
      for (uint16_t y = 0; y < kScreenHeight; y++)
        rgb24Buffer[XY(kScreenWidth - 1, y)].nscale8(scale);
    }



    // create a square twister to the left or counter-clockwise
    // x and y for center, r for radius
    // SpiralStream(32, 32, 16, 120);
    void SpiralStream(uint16_t x, uint16_t y, uint16_t r, uint8_t dim)
    {
      // from the outside to the inside
      // d = 32; d > 0; d--
      for (uint16_t d = r; d > 0; d--)
      {
        // i = 32 - 32 - 1; i < 32 + 32; i++
        // i = 32 - 0 - 1;  i < 32 + 0; i++)
        for (uint16_t i = x - d - 1; i < x + d; i++)
        {
          rgb24Buffer[XY(i, y - d)] += rgb24Buffer[XY(i + 1, y - d)]; // lowest row to the right
          rgb24Buffer[XY(i, y - d)].nscale8(dim);
        }

        for (uint16_t i = y - d; i < y + d; i++)
        {
          rgb24Buffer[XY(x + d, i)] += rgb24Buffer[XY(x + d, i + 1)]; // right colum up
          rgb24Buffer[XY(x + d, i)].nscale8(dim);
        }

        for (uint16_t i = x + d; i > x - d; i--)
        {
          rgb24Buffer[XY(i, y + d)] += rgb24Buffer[XY(i - 1, y + d)]; // upper row to the left
          rgb24Buffer[XY(i, y + d)].nscale8(dim);
        }

        for (uint16_t i = y + d; i > y - d; i--)
        {
          rgb24Buffer[XY(x - d, i)] += rgb24Buffer[XY(x - d, i - 1)]; // left colum down
          rgb24Buffer[XY(x - d, i)].nscale8(dim);
        }
      }
    }



    void CircleStream(uint8_t value)
    {
      rgb24DimAll(value);

      for (uint8_t offset = 0; offset < (uint8_t )kMatrixCenterX; offset++)
      {
        boolean hasprev = false;
        uint16_t prevxy = 0;

        for (uint8_t theta = 0; theta < 255; theta++)
        {
          uint16_t x1 = mapcos8(theta, offset, (kScreenWidth - 1 - 1) - offset);
          uint16_t y1 = mapsin8(theta, offset, (kScreenHeight - 1 - 1) - offset);
          uint16_t xy = XY(x1, y1);

          if (hasprev)
          {
            ledArray2[prevxy] += rgb24Buffer[xy];
          }

          prevxy = xy;
          hasprev = true;
        }
      }

      for (uint16_t i = 0; i < kScreenWidth; i++)
      {
        for (uint16_t j = 0; j < kScreenHeight; j++)
        {
          uint16_t xy = XY(i, j);
          rgb24Buffer[xy] = ledArray2[xy];
          rgb24Buffer[xy].nscale8(value);
          ledArray2[xy].nscale8(value);
        }
      }
    }



    // spread pixel horizontally
    void smearHorizontal(uint8_t scale)
    {

      for (uint16_t x = 0; x < kScreenWidth; x++)
      {
        for (uint16_t y = 0; y < kScreenHeight; y++)
        {
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x - 1, y)];
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x, y)];
          rgb24Buffer[XY(x, y)] += rgb24Buffer[XY(x + 1, y)];
          rgb24Buffer[XY(x, y)].nscale8(scale);
        }
      }
    }



    // rotates the first 16x16 quadrant 3 times onto a +90 degrees rotation for each one)
    void Caleidoscope1()
    {
      for (int x = 0; x < 32; x++)
      {
        for (int y = 0; y < 32; y++)
        {
          // 1st copy
          rgb24Buffer[XY(63 - x, y)] = rgb24Buffer[XY(x, y)];

          // 2nd copy
          rgb24Buffer[XY(63 - x, 63 - y)] = rgb24Buffer[XY(x, y)];

          // 3rd
          rgb24Buffer[XY(x, 63 - y)] = rgb24Buffer[XY(x, y)];
        }
      }
    }


    // mirror the first 16x16 quadrant 3 times onto a 32x32
    void Caleidoscope2()
    {
      for (uint16_t x = 0; x < 16; x++)
      {
        for (uint16_t y = 0; y < 16; y++)
        {
          rgb24Buffer[XY(31 - x, y)]  = rgb24Buffer[XY(y, x)];
          rgb24Buffer[XY(x, 31 - y)] = rgb24Buffer[XY(y, x)];
          rgb24Buffer[XY(31 - x, 31 - y)] = rgb24Buffer[XY(x, y)];
        }
      }
    }


    // copy one diagonal triangle into the other one within a 16x16
    void Caleidoscope3()
    {
      for (uint16_t x = 0; x < 16; x++)
      {
        for (uint16_t y = 0; y <= x; y++)
        {
          rgb24Buffer[XY(x, y)] = rgb24Buffer[XY(y, x)];
        }
      }
    }



    // copy one diagonal triangle into the other one within a 16x16 (90 degrees rotated compared to Caleidoscope3)
    void Caleidoscope4()
    {
      for (uint16_t x = 0; x < 16; x++)
      {
        for (uint16_t y = 0; y <= 16 - x; y++)
        {
          rgb24Buffer[XY(32 - y, 32 - x)] = rgb24Buffer[XY(x, y)];
        }
      }
    }



    // copy one diagonal triangle into the other one within a 8x8
    void Caleidoscope5()
    {
      for (uint16_t x = 0; x < 8; x++)
      {
        for (uint16_t y = 0; y <= x; y++)
        {
          rgb24Buffer[XY(x, y)] = rgb24Buffer[XY(y, x)];
        }
      }

      for (uint16_t x = 8; x < 16; x++)
      {
        for (uint16_t y = 16; y > 0; y--)
        {
          rgb24Buffer[XY(x, y)] = rgb24Buffer[XY(y, x)];
        }
      }
    }



    void Caleidoscope6()
    {
      for (uint16_t x = 1; x < 16; x++) //a
        rgb24Buffer[XY(17 - x, 7)] = rgb24Buffer[XY(x, 0)];

      for (uint16_t x = 2; x < 16; x++) //b
        rgb24Buffer[XY(17 - x, 6)] = rgb24Buffer[XY(x, 1)];

      for (uint16_t x = 3; x < 16; x++) //c
        rgb24Buffer[XY(17 - x, 5)] = rgb24Buffer[XY(x, 2)];

      for (uint16_t x = 4; x < 16; x++) //d
        rgb24Buffer[XY(17 - x, 4)] = rgb24Buffer[XY(x, 3)];

      for (uint16_t x = 5; x < 16; x++) //e
        rgb24Buffer[XY(17 - x, 3)] = rgb24Buffer[XY(x, 4)];

      for (uint16_t x = 6; x < 16; x++) //f
        rgb24Buffer[XY(17 - x, 2)] = rgb24Buffer[XY(x, 5)];

      for (uint16_t x = 7; x < 16; x++) //g
        rgb24Buffer[XY(17 - x, 1)] = rgb24Buffer[XY(x, 6)];
    }




    // copy x pixels from 0, startingY to 0, y + startingY + numY
    void mirrorDown(uint16_t startingX, uint16_t startingY, uint16_t numX, uint16_t numY)
    {
#ifdef CHECK_BOUNDS
      if (startingY > kScreenHeight - numY || startingX > kScreenWidth - numX )
      {
        Serial.println("mirrorDown Start Out of Bounds");
        return;
      }

      if (startingY + numY > kScreenHeight || startingX + numX > kScreenWidth)
      {
        Serial.println("mirrorDown Span Out of Bounds");
        return;
      }
#endif

      for (uint16_t x = startingX; x < startingX + numX; x++)
      {
        for (uint16_t y = startingY; y < startingY + numY; y++)
        {
          rgb24Buffer[XY(x, y + numY)] = rgb24Buffer[XY(x, y)];
        }
      }
    }





    void mirrorLeft(uint16_t startingX, uint16_t startingY, uint16_t numX, uint16_t numY)
    {
      for (uint16_t x = startingX; x < startingX + numX; x++)
      {
        for (uint16_t y = startingY; y < startingY + numY; y++)
        {
          rgb24Buffer[XY(x + numX, y)] = rgb24Buffer[XY(x, y)];
        }
      }
    }




    // copy top left 64x32 area down into lower left 64x32 area
    void mirror64down()
    {
      if (kScreenWidth < 64)
        return;

      for (uint16_t x = 0; x < 64; x++)
      {
        for (uint16_t y = 0; y < 64; y++)
        {
          rgb24Buffer[XY(x, y + 64)] = rgb24Buffer[XY(x, y)];
        }
      }
    }


    // copy left 64x64 pixels of matrix to next 64x64 pixels
    void mirror64Left()
    {
      if (kScreenWidth < 128)
        return;

      for (uint16_t x = 0; x < 64; x++)
      {
        for (uint16_t y = 0; y < 64; y++)
        {
          rgb24Buffer[XY(x + 64, y)] = rgb24Buffer[XY(x, y)];
        }
      }
    }



    // just move everything one line down
    void MoveDown()
    {
      for (uint16_t y = kScreenHeight - 1; y > 1; y--)
      {
        for (uint16_t x = 0; x < kScreenWidth; x++)
        {
          rgb24Buffer[XY(x, y)] = rgb24Buffer[XY(x, y - 1)];
        }
      }
    }

};
