/****************************************************
  Audio Patterns

  These are the various pattern functions, except
  for StarBurst which is a separte file. Patterns base on
  the "noise" function have issues with SM4.

  The life pattern currently consumes too much memory
  on displays larger than 128x128 so theres a #define
  to inclue it.

  conversion to SmartMatrix 4 continues...

    with rotation capability, new defs, example for 128x128 display
  matrix.getScreenWidth()                          num horizontal pixels (128)
  getScreenHeight()                                num vertical pixels   (128)
  kScreenWidth = matrix.getScreenWidth() - 1;      matrix indexing       (127)
  kScreenHeight = matrix.getScreenHeight() - 1 ;   matrix indexing       (127)
  kMatrixCenterX = kScreenWidth / 2;               horizontal center     (63)
  kMatrixCenterY = kScreenHeight / 2;              vertical center       (63)

*******************************************************/



#pragma once


#include "Effects.h"
Effects effects;

#include "fastMath.h"
#include "Vector.h"
#include "Boid.h"
#include "colorUtils.h"



extern void saveCurrentPattern(uint8_t patternNum);


// led array buffer
rgb24* rgb24Buffer;



// non-class/static globals
uint32_t start = 0;             // sleep timer start time
uint32_t sleepCount = 0;        // current sleep timer time
rgb24 colorsArray[8];           // temp array for colors
uint8_t lastPattern = 255;      // saves value of the last pattern used
rgb24 rectColor = WHITE;        // color for static rect pattern, default is white

// star pattern
#include "star.h"
Star stars[NUM_STARS];


// life pattern
#ifdef INCLUDE_LIFE
#include "Life.h"
Life life;
#endif



//----------------------------------
// in work due to rotation change,
// needs to be after audioPatterns.h
// and after rotation is updated

uint16_t const numBoids = kMatrixWidth + 16;
Boid boids[numBoids];
//----------------------------------




// funky way to select pattern, but makes it
// easy to modify during development
enum {
  DISPLAYOFF,
  RECTS,
  ANALYZER7,
  ANALYZER16,
  ANALYZERRAINDROP,
  STARBURST,
  BOUNCE,
  STARS1,
  STARS2,
  SPIRO,
  PLASMA1,
  PLASMA2,
  ANALYZERPIXELS,
  LINECHART,
  KALEIDO1,
  KALEIDO2,
  KALEIDO3,
  RADIALPIXELS,
  FALLINGSPECTRO,
  LINESTOOUTSIDE,
  SINEWAVE,
  SPIRAL,
  INCREMENTALDRIFT,
  RADIALTEST,
#ifdef INCLUDE_LIFE
  LIFE,   // currently must be last pattern
#endif
  LAST
};


const uint8_t numPatterns = LAST;


String patternName[numPatterns] = {
  "off",              // 0
  "rects",            // 1
  "analyzer7",        // 2
  "analyzer16",       // 3
  "analyzerRaindrop", // 4
  "starBurst",        // 5
  "bounce",           // 6
  "stars1",           // 7
  "stars2",           // 8
  "spiro",            // 9
  "plasma1",          // 10
  "plasma2",          // 11
  "analyzerUpRight",  // 12
  "lineChart",        // 13
  "kaleido1",         // 14
  "kaleido2",         // 15
  "kaleido3",         // 16
  "radialPixels",     // 17
  "fallingSpectro",   // 18
  "linesToOutside",   // 19
  "sineWave",         // 20
  "spiral",           // 21
  "incrementalDrift", // 22
  "radialTest",       // 23
#ifdef INCLUDE_LIFE
  "life"              // 24
#endif
};



//--------------------------------------------------------------------------------


class AudioPatterns {

  private:
    float  beta = 0;                // angle used in several patterns
    float  avg = 0;                 // used in radialCircles
    uint8_t hueOffset = 0;          // color shift var
    uint16_t lastX = 0;             // general purpose var
    uint16_t lastY = 0;             // general purpose var
    uint32_t counts = 0;            // general purpose counter

    // scaling tools
    uint16_t Y_AUDIO_SF;
    uint16_t X_PIXELS_PER_BAND7;
    uint16_t X_PIXELS_PER_BAND16;

    // spiro pattern
    uint8_t theta1 = 0;              // angle used in several patterns
    uint8_t theta2 = 0;              // angle used in several patterns
    uint8_t rotate = 0;              // used in radialCircles()
    uint8_t radius = 4;
    uint8_t minR;
    uint8_t maxR;
    uint8_t spiroCount = 1;
    uint8_t spiroOffset = 256 / spiroCount;
    boolean spiroIncrement = false;
    boolean handledChange = false;

    // boids pattern
    PVector gravity;                // boid vars
    PVector impulse;                // boid vars

    // radial circles pattern
    uint8_t lastColorIndex = 0;     // last color used in radial circles

    void Life();

  public:
    bool initialized = false;       // flag used to initialize pattern
    bool revertColors = false;      // used in radial circles



    void init()
    {
      initialized = false;
      hueOffset = 0;
      gravity = PVector(0, 0.0250);
      impulse = PVector(0, 0);

      Serial.print("Pattern: ");
      Serial.print(pattern);
      Serial.print(" = ");
      Serial.println(patternName[pattern]);

      X_PIXELS_PER_BAND7 = matrix.getScreenWidth() / EQ_BANDS7;
      X_PIXELS_PER_BAND16 = matrix.getScreenWidth() / EQ_BANDS16;
      Y_AUDIO_SF = (MAX_AUDIO + 1) / (matrix.getScreenHeight() + 1);
    }


    void update()
    {
      // get audio data
      readAudio();

      // dim display if no audio
      if (gain == maxGain)
      {
        sleepCount++;

        if (brightness > 0 && sleepCount > 4000)
          brightness -= 1;
      }
      else
      {
        sleepCount = 0;
        if (brightness < 255)
          brightness += 1;
      }


      // periodically check if its time to increment pattern
      // but don't switch while sleeping
      if (autoincrement && brightness > 64)
      {
        if (millis() - lastSwitch > AUTO_SWITCH_DURATION)
        {
          pattern++;
          if (pattern > numPatterns - 1)
            pattern = 1;
          lastSwitch = millis();

          Serial.print("Pattern = ");
          Serial.print(pattern);
          Serial.print(" = ");
          Serial.println(patternName[pattern]);
        }
      }

      // did we change patterns?
      if (pattern != lastPattern)
      {
        // set eepromUpdate flag
        eepromUpdate = true;

        // re-initialize pattern vars
        init();
      }

      // update buffers before updating pattern
      rgb24Buffer = backgroundLayer.backBuffer();

      // draw audio pattern
      switch (pattern)
      {
        case DISPLAYOFF: off(); break;
        case RECTS: rects(); break;
        case ANALYZER7: analyzer7(); break;
        case ANALYZER16: analyzer16(); break;
        case ANALYZERRAINDROP: analyzerRaindrop(); break;
        case STARBURST: starBurst(); break;
        case BOUNCE: bounce(); break;
        case STARS1: stars1(); break;
        case STARS2: stars2(); break;
        case SPIRO: spiro(); break;
        case PLASMA1: plasma1(); break;
        case PLASMA2: plasma2(); break;
        case ANALYZERPIXELS: analyzerPixels(); break;
        case LINECHART: lineChart(); break;
        case KALEIDO1: kaleido1(); break;
        case KALEIDO2: kaleido2(); break;
        case KALEIDO3: kaleido3(); break;
        case RADIALPIXELS: radialPixels(); break;
        case FALLINGSPECTRO: fallingSpectro(); break;
        case LINESTOOUTSIDE: linesToOutside(); break;
        case SINEWAVE: sineWave(); break;
        case SPIRAL: spiral(); break;
        case INCREMENTALDRIFT: incrementalDrift(); break;
        case RADIALTEST: radialTest(); break;
#ifdef INCLUDE_LIFE
        case LIFE: life.updateWorld(); break;
#endif

        default:
          Serial.println("Invalid Pattern Selected");
          pattern = 0;
          return;
      }

      // remember the pattern
      lastPattern = pattern;

      // special cases for now, updated in pattern
      if (pattern == STARBURST)
        return;

#ifdef INCLUDE_LIFE
      if (pattern == LIFE)
        return;
#endif

      // update brightness
      matrix.setBrightness(brightness);

      // swap in new screen
      backgroundLayer.swapBuffers();

      // get updated screen buffer
      rgb24Buffer = backgroundLayer.backBuffer();
    }


    void off()
    {
      if (!initialized)
        backgroundLayer.fillScreen(BLACK);

      initialized = true;
    }



    // general purpose non-audio pattern
    void rects()
    {
      if (!initialized)
      {
        backgroundLayer.fillScreen(BLACK);
        initialized = true;

        rgb24 color = rectColor;
        float intensity = 250;

        for (uint16_t i = 0; i < matrix.getScreenHeight() / 12; i++)
        {
          uint16_t vOffset1 = i * 2;
          uint16_t hOffset1 = i * 2;
          uint16_t vOffset2 = i * 2;
          uint16_t hOffset2 = i * 2;

          backgroundLayer.drawRectangle(i + hOffset1, i + vOffset1, (kScreenWidth - 1) - hOffset2, kScreenHeight - vOffset2, color);

          intensity = intensity * 0.95;
          color = rgb24SetColorBrightness(color, (uint8_t )intensity);
        }
      }

      sleepCount = 0;
      brightness = 250;
    }


    // base 7-channel EQ display
    void analyzer7()
    {
      initialized = true;
      backgroundLayer.fillScreen(BLACK);

      for (int x = 0; x < kScreenWidth; x++)
      {
        int band = x / X_PIXELS_PER_BAND7;
        int level = peaks[band] / Y_AUDIO_SF;
        if (level > kScreenHeight)
          level = kScreenHeight;

        // show unscaled levels for audio input testing
        if (testMode)
          level = rawAudio[band] / Y_AUDIO_SF;

        backgroundLayer.drawLine(x, kScreenHeight, x, kScreenHeight - level - level, rgb24Colors8[band]);
      }
    }


    // EQ display for interpolated 16 EQ channels
    void analyzer16()
    {
      initialized = true;
      backgroundLayer.fillScreen(BLACK);

      for (int x = 0; x < kScreenWidth; x++)
      {
        int band = x / X_PIXELS_PER_BAND16;
        if (band < EQ_BANDS16)
        {
          int level = audio16[band] / Y_AUDIO_SF;
          backgroundLayer.drawLine(x, kScreenHeight, x, kScreenHeight - level, rgb24Colors16[band]);
        }
      }
    }



    void analyzerRaindrop()
    {
      uint16_t x = 0;
      initialized = true;

      rgb24DimAll(240);

      for (uint16_t index = 0; index < EQ_BANDS16; index++)
      {
        // calc x-axis position
        x = index * X_PIXELS_PER_BAND16;

        // scale to prevent clipping
        int16_t level = audio16[index] / (Y_AUDIO_SF + 2); // smaller divisor = more activity

        backgroundLayer.drawLine(x, kScreenHeight - level - 1, x, kScreenHeight, rgb24Colors16[index]);

        //  subtract the height of the length (number of leds) of the raindrop
        // and then draw over the line with a black line
        level -= 5;
        if (level < 0)
          level = 0;

        backgroundLayer.drawLine(x, kScreenHeight - 1 - level, x, kScreenHeight - 1, BLACK);

        // add bottom row of always on leds to create a mirror-like effect
        if (audio16[index] > 0)
          backgroundLayer.drawPixel(x, kScreenHeight - 1, rgb24Colors16[index]);
        else
          backgroundLayer.drawPixel(x, kScreenHeight - 1, BLACK);
      }
    }


    void starBurst()
    {
      if (!initialized)
      {
        // initialize the stars
        Serial.print("num stars "); Serial.println(NUM_STARS);

        for (uint16_t i = 0; i < NUM_STARS; i++)
          stars[i] = Star();

        Serial.println("stars initialized");
        initialized = true;
      }

      // process starfield
      for (uint16_t i =  0; i < NUM_STARS; i++)
      {
        // move star if alive
        if (stars[i].isAlive())
          stars[i].tick();
        else
          stars[i].starBirth();
      }

      // dim all stars to create trails
      rgb24DimAll(persistance);

      // refresh the led matrix
      backgroundLayer.swapBuffers();

      delay(5);
    }



    void stars1()
    {
      if (!initialized)
      {
        backgroundLayer.fillScreen(BLACK);
        initialized = true;

      }

      rgb24DimAll(250);

      if (maxLevel > 200)
      {
        rgb24 color = rgb24Colors8[maxBand];

        for (uint8_t i = 0; i < 8; i++)
          backgroundLayer.drawPixel(random(0, kScreenWidth), random(0, kScreenHeight), color);

        delay(20);
      }
    }



    void stars2()
    {
      if (!initialized)
      {
        backgroundLayer.fillScreen(BLACK);
        initialized = true;
      }



      // add some smear - dosen't work after rotation mode
      //effects.StreamRight(120);
      //doing this instead
      effects.MoveDown();
      rgb24DimAll(250);


      // make a few random stars if audio above threshold
      if (maxLevel > 200)
      {
        rgb24 color = rgb24Colors8[maxBand];

        for (uint8_t i = 0; i < 8; i++)
          backgroundLayer.drawPixel(random(0, kScreenWidth), random(0, kScreenHeight), color);
      }

      delay(20);
    }



    void bounce()
    {
      uint16_t height = kScreenHeight;
      uint16_t offset = 0;

      // on big displays, show pattern in middle
      if (kScreenHeight > 64)
      {
        height = kScreenHeight / 2;
        offset = kScreenHeight / 4;
      }

      if (!initialized)
      {
        initialized = true;

        int direction = random(0, 2); // -1, 1
        if (direction == 0)
          direction = -1;

        for (uint16_t i = 0; i < numBoids; i++)
        {
          Boid boid = Boid(i, height);
          boid.velocity.x = 0;
          boid.velocity.y = 0;
          boid.maxforce = 10;
          boid.maxspeed = 10;
          boid.limitX = kScreenWidth;
          boid.limitY = height;

          boids[i] = boid;
        }
      }

      // dim all pixels on the display
      rgb24DimAll(170);

      for (uint16_t i = 0; i < kScreenWidth; i++)
      {
        Boid boid = boids[i];
        boid.velocity.x = 0;
        boid.location.x = i;
        boid.applyForce(gravity * 2);

        uint16_t bandIndex = i / X_PIXELS_PER_BAND16;
        float level = audio16[bandIndex];

        if (boid.location.y == height - 1)
        {
          impulse.y = randomf(level / 1.5, level) * -1 / 420.0;  // was 960.0
          boid.applyForce(impulse);
        }

        boid.update();
        boid.bounceOffBorders(0.2);
        boids[i] = boid;

        backgroundLayer.drawPixel(boid.location.x, boid.location.y + offset, rgb24Colors16[bandIndex]);
        delayMicroseconds(30);
      }

      // pattern speed delay
      delay(5);
    }



    void spiro()
    {
      if (!initialized)
      {
        theta1 = 0;
        theta2 = 0;
        initialized = true;
      }

      rgb24DimAll(251);
      boolean change = false;

      minR = kMatrixCenterY - radius;
      maxR = kMatrixCenterY + radius + 1;

      for (int i = 0; i < spiroCount; i++)
      {
        uint8_t x = mapsin8(theta1 + i * spiroOffset, minR, maxR);
        uint8_t y = mapcos8(theta1 + i * spiroOffset, minR, maxR);

        uint8_t x2 = mapsin8(theta2 + i * spiroOffset, x - radius, x + radius);
        uint8_t y2 = mapcos8(theta2 + i * spiroOffset, y - radius, y + radius);

        backgroundLayer.drawPixel(x2, y2, rgb24Colors8[pkBand]);

        // check if spiros are in center
        if ((x2 == kMatrixCenterX     && y2 == kMatrixCenterY) ||
            (x2 == kMatrixCenterX - 1 && y2 == kMatrixCenterY - 1))
          change = true;
      }

      theta2 += 2;  // default= 1

      if (change == true)
      {
        if (pkLevel > 300)
        {
          //printValue("maxLevel", maxLevel);
          if (radius < kScreenWidth / 4 - 2)
            radius += 2;
        }
        else if (radius > 5)
          radius -= 2;
      }


      EVERY_N_MILLIS(25) {  // default = 25
        theta1 += 1;
      }

      EVERY_N_MILLIS(25) {  // 100
        if (change && !handledChange)
        {
          handledChange = true;

          if (spiroCount < 4 && pkLevel > 300)
            spiroCount += 1;

          else if (spiroCount < 32 && pkBand > 2 && pkLevel > 300)
            spiroCount *= 2;

          else if (spiroCount > 1 && pkBand < 3)
          {
            if (spiroCount > 4)
              spiroCount /= 2;
            else
              spiroCount -= 1;
          }

          spiroOffset = 256 / spiroCount;
        }

        if (!change)
          handledChange = false;
      }
    }



    void plasma1()
    {
      initialized = true;
      uint8_t scale = peaks[5] / 16 + 16;

      // position in the noise space depending on band 5
      // = change of the pattern
      effects.noiseZ = avgLevel / 5;  // was peaks[5] / 4;

      // x scrolling through
      // = horizontal movement
      effects.noiseX = effects.noiseX + 50;

      // y controlled by lowest band
      // = jumping of the pattern
      effects.noiseY = peaks[0] / 8;

      // calculate the noise array
      effects.FillNoiseCentral(scale);

      for (uint16_t i = 0; i < kScreenWidth; i++)
      {
        for (uint16_t j = 0; j < kScreenHeight; j++)
        {
          uint8_t pos = effects.noise[i][j];
          backgroundLayer.drawPixel(i, j, wheel8(pos));
        }
      }
      delay(10);
    }




    void plasma2()
    {
      initialized = true;
      uint16_t level0 = audio[0] / 10;
      uint16_t level5 = audio[5] / 10;


      // fixed zoom factor
      uint16_t scale = 30 + level0 / 4;

      // position in the noise space depending on band 5
      // = change of the pattern
      effects.noiseZ = level0;

      // x scrolling through
      // = horizontal movement
      effects.noiseX = effects.noiseX - 20 + avgLevel / 8;

      // = jumping of the pattern
      effects.noiseY = level5 / 2;

      // calculate the noise array
      effects.FillNoiseCentral(scale);

      // map the noise
      for (uint16_t i = 0; i < kScreenWidth; i++)
      {
        for (uint16_t j = 0; j < kScreenHeight; j++)
        {
          uint8_t c = effects.noise[i][j] * 3 / 2;
          backgroundLayer.drawPixel(i, j, wheel8(c));
        }
      }
      delay(20);
    }



    // main analyzer drawing routine
    void drawAnalyzerPixels(uint16_t width, uint16_t height)
    {
      for (uint16_t x = 0; x < width; x++)
      {
        uint8_t band = x / X_PIXELS_PER_BAND16;
        uint16_t level = audio16[band] / Y_AUDIO_SF;
        uint16_t y = height - 1 - level;
        backgroundLayer.drawPixel(x, y, rgb24Colors16[band]);
      }
    }


    void analyzerPixels()
    {
      initialized = true;
      effects.StreamUpAndRight(120);
      drawAnalyzerPixels(kScreenWidth, kScreenHeight);
    }



    // main analyzerLines drawing routine
    void drawAnalyzerLines()
    {
      initialized = true;

      for (uint8_t i = 0; i < EQ_BANDS16 - 1; i++)
      {
        rgb24 color = rgb24Colors16[i + 1];

        uint16_t level = audio16[i] / Y_AUDIO_SF;
        uint16_t nextLevel = audio16[i + 1] / Y_AUDIO_SF;

        uint16_t y = kScreenHeight - 1 - level;
        uint16_t nextY = kScreenHeight - 1 - nextLevel;

        //y = y >= MATRIX_HEIGHT ? MATRIX_HEIGHT - 1 : y;
        //nextY = nextY >= MATRIX_HEIGHT ? MATRIX_HEIGHT - 1 : nextY;
        uint16_t length = kScreenWidth / 16;

        backgroundLayer.drawLine(i * length,  y, (i * length) + length,  nextY, color);
      }
    }



    void lineChart()
    {
      initialized = true;
      backgroundLayer.fillScreen(BLACK);
      drawAnalyzerLines();
    }




    void kaleido1()
    {
      uint8_t index;

      if (!initialized)
      {
        backgroundLayer.fillScreen(BLACK);
        initialized = true;
      }

      if (audio[1] > 500)
      {
        index = hueOffset % 8;
        //printValue("index1", index);
        backgroundLayer.drawPixel(1, 1, rgb24Colors8[index]);
        backgroundLayer.drawPixel(5, 5, rgb24Colors8[index]);
      }

      if (audio[3] > 400)
      {
        index = (hueOffset + 85) % 8;
        //printValue("index2", index);
        backgroundLayer.drawPixel(10, 10, rgb24Colors8[index]);
        backgroundLayer.drawPixel(16, 16, rgb24Colors8[index]);
      }

      if (audio[5] > 400)
      {
        index = (hueOffset + 170) % 8;
        //printValue("index3", index);
        backgroundLayer.drawPixel(20, 20, rgb24Colors8[index]);
        backgroundLayer.drawPixel(28, 28, rgb24Colors8[index]);
      }

      effects.updateBuffer();

      effects.SpiralStream(32, 32, 32, 128);
      effects.Caleidoscope1();
      effects.mirrorDown(0, 0, 0, 32);
      effects.mirrorDown(0, 0, 64, 64);
      effects.mirrorLeft(0, 0, 64, 128);
      effects.mirrorLeft(0, 0, 64, 128);

      EVERY_N_MILLIS(100)
      {
        hueOffset++;
      }
    }



    // spectrum mandala, color linked to osci (MSGEQtest8)
    void kaleido2()
    {
      uint8_t index;

      if (!initialized)
      {
        backgroundLayer.fillScreen(BLACK);
        initialized = true;
      }

      if (audio[1] > 400)
      {
        index = hueOffset % 8;
        //printValue("index1", index);
        backgroundLayer.drawPixel(1, 1, rgb24Colors8[index]);
        backgroundLayer.drawPixel(3, 7, rgb24Colors8[index]);
        backgroundLayer.drawPixel(7, 13, rgb24Colors8[index]);
        backgroundLayer.drawPixel(12, 18, rgb24Colors8[index]);
      }

      if (audio[3] > 400)
      {
        index = (hueOffset + 85) % 8;
        //printValue("index2", index);
        backgroundLayer.drawPixel(8, 10, rgb24Colors8[index]);
        backgroundLayer.drawPixel(10, 16, rgb24Colors8[index]);
        backgroundLayer.drawPixel(20, 16, rgb24Colors8[index]);

      }

      if (audio[5] > 400)
      {
        index = (hueOffset + 170) % 8;
        //printValue("index3", index);
        backgroundLayer.drawPixel(10, 3, rgb24Colors8[index]);
        backgroundLayer.drawPixel(20, 20, rgb24Colors8[index]);
        backgroundLayer.drawPixel(28, 22, rgb24Colors8[index]);
        backgroundLayer.drawPixel(28, 12, rgb24Colors8[index]);
      }

      effects.updateBuffer();

      effects.SpiralStream(32, 32, 32, 128);
      effects.Caleidoscope1();
      effects.mirrorDown(0, 0, 0, 32);
      effects.mirrorDown(0, 0, 64, 64);
      effects.mirrorLeft(0, 0, 64, 128);
      effects.mirrorLeft(0, 0, 64, 128);

      EVERY_N_MILLIS(100)
      {
        hueOffset++;
      }
    }




    void kaleido3()
    {
      uint16_t x1, x2, y;
      uint8_t color;
      int level;

      initialized = true;

      // EQ_BANDS7 = 7
      for (int i = 0; i < EQ_BANDS7; i++)
      {
        level = peaks[i];
        if (level < 400)
          level = 0;

        // draw pattern
        x1 = 14 - (i * 2);           // 14 -> 2
        x2 = 15 - (i * 2);           // 15 -> 3
        y = 16 - (level / 64);       // 0 -> 32
        color = maxBand * 2;         // should be < 16

        //if (printLevel > 2)
        //{
        //  printValue("x1", x1);
        //  printValue("x2", x2);
        //  printValue("y", y);
        //  printValue("color", color);
        //  Serial.println();
        //}

        // pixel(x, y, color)
        backgroundLayer.drawPixel(x1, y, rgb24Colors16[color]);
        backgroundLayer.drawPixel(x2, y, rgb24Colors16[color]);
        backgroundLayer.drawPixel(x1 + 8, y + 8, rgb24Colors16[color]);
        backgroundLayer.drawPixel(x2 + 8, y + 8, rgb24Colors16[color]);
      }

      effects.updateBuffer();

      effects.Caleidoscope5();
      effects.Caleidoscope1();
      effects.mirrorDown(0, 0, 64, 64);
      effects.mirrorLeft(0, 0, 64, 64);
      effects.mirrorLeft(0, 64, 64, 64);
      rgb24DimAll(240);
    }




    void radialPixels()
    {
      uint16_t level = 0;
      rgb24 color;
      uint8_t localtheta = 0;

      initialized = true;

      rgb24DimAll(230);

      for (uint8_t i = 0; i < EQ_BANDS7; i++)
      {
        level = peaks[i];

        //if (i == 0) printValue("level[0]", level);

        if (i == 0 && level > 600)
          theta1 -= 1;

        color = wheel8(level / 4);
        localtheta = theta1 - (i * 2) * (256 / 14);

        uint16_t x = mapcos8(localtheta, 4, kScreenWidth - 1 - 4);
        uint16_t y = mapsin8(localtheta, 4, kScreenHeight - 1 - 4);
        rgb24Buffer[XY(x, y)] = color;

        x = mapcos8(localtheta, 12, kScreenWidth - 1 - 12);
        y = mapsin8(localtheta, 12, kScreenHeight - 1 - 12);
        rgb24Buffer[XY(x, y)] = color;

        localtheta = theta1 - (i * 2 + 1) * (256 / 14);

        x = mapcos8(localtheta, 4, kScreenWidth - 1 - 4);
        y = mapsin8(localtheta, 4, kScreenHeight - 1 - 4);
        rgb24Buffer[XY(x, y)] = color;

        x = mapcos8(localtheta, 12, kScreenWidth - 1 - 12);
        y = mapsin8(localtheta, 12, kScreenHeight - 1 - 12);
        rgb24Buffer[XY(x, y)] = color;
      }
    }



    void fallingSpectro()
    {
      initialized = true;

      effects.updateBuffer();
      effects.MoveDown();

      for (uint16_t x = 0; x < kScreenWidth; x++)
      {
        uint16_t level = audio16[x * 16 / kScreenWidth] / 4;
        level =  constrain(level, 0, 255);
        uint8_t wheelPos = x * 256 / kScreenWidth;

        //printValue("level", level);
        //printValue("wheelPos", wheelPos);

        rgb24 color = wheel8(wheelPos);
        color = rgb24SetColorBrightness(color, level);
        rgb24Buffer[XY(x, 1)] = color;
      }
    }



    // this only works properly on a square display
    void linesToOutside()
    {
      initialized = true;

      rgb24DimAll(235);
      //effects.SpiralStream(15, 15, 16, 120);

      uint16_t x0 = kMatrixCenterX;
      uint16_t y0 = kMatrixCenterY;
      uint16_t linesPerBand = 1;

      float degreesPerLine = 360 / (EQ_BANDS7 * linesPerBand);
      float angle = 360.0;

      for (uint8_t band = 0; band < EQ_BANDS7; band++)
      {
        int level = audio[band] / 4;

        for (uint16_t i = 0; i < linesPerBand; i++)
        {
          rgb24 color = wheel8(level / 4);
          float length = level / (kMatrixCenterY / 2);

          if (length > 0.0)
          {
            float radians = radians(angle);
            uint16_t x1 = x0 + length * sin(radians);
            uint16_t y1 = y0 + length * cos(radians);

            radians = radians(angle - degreesPerLine);
            uint16_t x2 = x0 + length * sin(radians);
            uint16_t y2 = y0 + length * cos(radians);

            // printValue("x0", x0);
            // printValue("y0", y0);
            // printValue("x1", x1);
            // printValue("y1", y1);
            // printValue("x2", x2);
            // printValue("y2", y2);

            backgroundLayer.fillTriangle(x0, y0, x1, y1, x2, y2, color);
          }

          angle -= degreesPerLine;
        }
        delay(100);
      }
    }



    void sineWave()
    {
      initialized = true;

      rgb24DimAll(170);

      for (uint16_t x = 0; x < kScreenWidth; x++)
      {
        uint8_t colorIndex = maxBand;
        uint16_t level = kScreenHeight * avgLevel / 300;
        level = constrain(level, 10, kScreenHeight / 2);
        uint16_t y = beatsin8(x / 2, 0, level);

        backgroundLayer.drawPixel(x, y + kScreenHeight / 4, rgb24Colors8[colorIndex]);
      }
    }



    void spiral()
    {
      uint8_t dim = beatsin8(2, 230, 250);

      initialized = true;

      rgb24DimAll(dim);

      for (uint16_t i = 0; i < kScreenWidth / 2; i++)
      {
        uint16_t val = avgLevel / 10;
        if (val < 32) val = 32;

        uint16_t x = beatcos8((val - i) * 2, kMatrixCenterX - i, kMatrixCenterX + i);
        uint16_t y = beatsin8((val - i) * 2, kMatrixCenterY - i, kMatrixCenterY + i);

        if (printLevel > 2)
        {
          printValue("x", x);
          printValue("y", y);
        }

        backgroundLayer.drawPixel(x, y, rgb24Colors8[(uint8_t )avgBand]);
      }
    }



    void incrementalDrift()
    {
      uint8_t dim = beatsin8(2, 170, 250);
      initialized = true;

      rgb24DimAll(dim);

      for (uint16_t i = 0; i < kScreenWidth; i++)
      {
        uint16_t x = 0;
        uint16_t y = 0;

        uint8_t bandIndex = 0;

        if (i < 16)
        {
          x = beatcos8((i + 1) * 2, i, (kScreenWidth - 1 - 1) - i);
          y = beatsin8((i + 1) * 2, i, (kScreenHeight - 1 - 1) - i);
          bandIndex = i / 2;
        }
        else
        {
          x = beatsin8((kScreenWidth - 1 - i) * 2, (kScreenWidth - 1 - 1) - i, i);
          y = beatcos8((kScreenWidth - 1 - i) * 2, (kScreenHeight - 1 - 1) - i, i);
          bandIndex = (31 - i) / 2;
        }

        if (bandIndex > EQ_BANDS7)
          bandIndex = EQ_BANDS7;

        backgroundLayer.drawPixel(x, y, rgb24Colors8[maxBand]);
      }
    }




    void radialTest()
    {
      rgb24DimAll(254);
      initialized = true;

      for (uint16_t offset = 0; offset < kMatrixCenterX; offset++)
      {
        uint8_t hue = 255 - (offset * 16 + hueOffset);
        rgb24 color = wheel8(hue);
        uint16_t x = mapcos8(theta1, offset, (kScreenWidth - 1 - 1) - offset);
        uint16_t y = mapsin8(theta1, offset, (kScreenHeight - 1 - 1) - offset);
        uint16_t xy = XY(x, y);
        rgb24Buffer[xy] = color;

        EVERY_N_MILLIS(25)
        {
          theta1 += 2;
          hueOffset += 1;
        }
      }
    }


    // special purpose pattern to test max current draw
    void white()
    {
      // fill with white
      backgroundLayer.fillScreen(WHITE);
      backgroundLayer.swapBuffers();

      delay(5000);

      backgroundLayer.fillScreen(BLACK);
      backgroundLayer.swapBuffers();
    }
};
