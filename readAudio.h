/*************************************************************

   audio.h - reads the audio level for each of 7 eq bands from
   a MSGEQ07 IC. Thre is an AGC that adjusts the audio gain to
   conpensate for different volume levels. See the global defs
   for the various values returned.
   Returns either 7, 8, or 16 EQ band values

   MSGEQ07 Timing Requirements:
   tr - Reset Pulse Width:    100 nS min
   trs - Reset to Strobe:     72 uS min
   ts - Strobe Pulse Width:   18 uS min
   tss - Strobe to Strobe:    72 uS min
   to - Output Settling Time: 36 uS min

   MSGEQ07 band centers:
   1: 63 Hz
   2: 160 Hz
   3: 400 Hz
   4: 1 kHz
   5: 2.5 kHz
   6: 6.25 kHz
   7: 16 kHz


   version 3.5   Jun2022  tbitson

 ************************************************/


#pragma once


#define MAX_AUDIO 1023
//#define CAL_MODE


// prototypes
void initAudio();
void readAudio();
void getAudioData();
void adjustGain();
void calcAvg();
void findPeaks();
void interpolate();
void printAudioValues();



// ----------------------------------------------
// adjust these to your preference

const bool useLogScale       = false;
const float maxGain          = 12.0;
const float minGain          = 0.1;
const uint16_t lowThreshold  = 450;
const uint16_t highThreshold = 630;
const int peakDecay          = 7;
const float minThreashold    = 30;
const float avgFactor        = 0.95;
bool usePeaks                = true;

// ----------------------------------------------


// constants
// The MSGEQ7 ic uses has 7 internal EQ bands (0 -> 6) which
// are read using teensy's 10-Bit (0-1023) A to D
// which are interpolated into 16 bands
const uint8_t EQ_BANDS7 = 7;
const uint8_t EQ_BANDS8 = 8;
const uint8_t EQ_BANDS16 = 16;

// global results from chip
float rawAudio[EQ_BANDS7];      // audio levels for the 7 eq bands UNSCALED
float audio[EQ_BANDS7];         // audio levels for the 7 eq bands scaled
float peaks[EQ_BANDS7];         // peak audio values for each of the 7eq band
float audio16[EQ_BANDS16];      // audio levels for the 16 interpolated bands


// global results (calculated)
float   maxLevel = 0;           // max level in all 7 bands
uint8_t maxBand = 0;            // band that the max level is in
float   pkLevel = 0;            // max peak level in all 7 bands
uint8_t pkBand = 0;             // band that the max peak level is in
float   maxRaw = 0;             // max of the 7 raw audio level. No gain applied
float   avgLevel = 0;           // avg audio level for all 7 bands
float   avgBand = 0;            // running avg of maxBand


// adjusts how much gain is needed to keep display in range
// when using linear scale (< 1.0 decreases gain). It is adjusted
// dynamically based on input level. There are a couple of settings above
float gain;

// values subtracted for each band to minimize background noise
// should be tailored for each installation - see hardware.h
float background[EQ_BANDS7] = BACKGROUND_OFFSET;

// enabled from serial to show audio data
bool audioDebug = false;




void initAudio()
{
  // power up the chip if using i/o pins for power
#ifdef MSGEQ7_VCC_PIN
  pinMode(MSGEQ7_GND_PIN, OUTPUT);
  digitalWrite(MSGEQ7_GND_PIN, LOW);
  delay(1);
  pinMode(MSGEQ7_VCC_PIN, OUTPUT);
  digitalWrite(MSGEQ7_VCC_PIN, HIGH);
  delay(500);
#endif


  // setup & init the MSGEQ7
  pinMode(MSGEQ7_RESET_PIN, OUTPUT);
  pinMode(MSGEQ7_STROBE_PIN, OUTPUT);
  
  digitalWrite(MSGEQ7_RESET_PIN, LOW);
  digitalWrite(MSGEQ7_STROBE_PIN, HIGH);

  // starting gain
  if (useLogScale)
  {
    gain = 1.0;
    Serial.println("using Log Audio Scale");
  }
  else
  {
    gain = 4.0;
    Serial.println("using Linear Audio Scale");
  }
}





void readAudio()
{
  getAudioData();
  adjustGain();
  calcAvg();
  findPeaks();
  interpolate();

  if (printLevel > 2)
    printAudioValues();
}





void getAudioData()
{
  float value;

  // reset MSEG07 for data read, 100ns min
  digitalWrite(MSGEQ7_RESET_PIN, HIGH);
  delayMicroseconds(25);
  digitalWrite(MSGEQ7_RESET_PIN, LOW);
  delayMicroseconds(19);

  maxLevel = 0;
  maxRaw   = 0;
  maxBand  = 0;

  // read the MSGEQ7 value for all 7 EQ bands (0 - 6)
  for (uint8_t band = 0; band < EQ_BANDS7; band++)
  {
    digitalWrite(MSGEQ7_STROBE_PIN, LOW);
    delayMicroseconds(25);

    // read audio band votage
    value = (float)analogRead(MSGEQ7_AUDIO_PIN);

    if (simAudio)
    {
      // sim data for 8 seconds then 2 seconds of silence
      if (millis() % 10000 < 2000)
        value = 0;
      else
        value = (float)random(100, 700);
    }

    // save raw value
    rawAudio[band] = value;

    // find max raw value
    if (value > maxRaw)
      maxRaw = value;

    // calibration test mode - display raw
    if (testMode)
    {
      Serial.print("   Band["); Serial.print(band); Serial.print("] = "); Serial.print(value);
      if (band == 6)
      {
        Serial.println();
        delay(50);
      }
    }

    // remove background noise
    value -= (float)background[band];

    // check if below minimum threshold (removes negative values)
    if (value < minThreashold)
    {
      value = 0;
    }
    else
    {
      // scale it
      if (useLogScale)
      {
        if (value > 0)
          value = log(value * gain / 2) * 100.0;
      }
      else
      {
        // linear scaling
        value = value * gain;
      }

      // bound it
      value = constrain(value, 0, 1000);

      // find max value and it's band
      if (value > maxLevel)
      {
        maxLevel = value;
        maxBand = band;
      }
    }

    // save it to array
    audio[band] = value;

    if (audioDebug)
      printAudioValues();


    // toggle msgeg7 to next band
    digitalWrite(MSGEQ7_STROBE_PIN, HIGH);
    delayMicroseconds(10);
  }
}




void adjustGain()
{
  // adjust audio gain based on max level detected
  if (maxLevel < lowThreshold && gain < maxGain)
    gain += 0.05;
  else if (maxLevel > highThreshold && gain > minGain)
    gain -= 0.20;
}



void calcAvg()
{
  // first, avg all 7 bands
  float sum = 0;
  for (uint8_t band = 0; band < EQ_BANDS7; band++)
    sum += audio[band];

  float newAvg = sum / (float)EQ_BANDS7;
  // then calc running average
  avgLevel = avgLevel * avgFactor + (1.0 - avgFactor) * newAvg;
  //printValue("avgLevel", avgLevel);

  avgBand = avgBand * avgFactor + (1.0 - avgFactor) * (float)maxBand;
  //printValue("avgBand", avgBand);
}


void findPeaks()
{
  float value;
  pkLevel = 0;

  //  check for new peak for each hw band
  for (uint8_t band = 0; band < EQ_BANDS7; band++)
  {
    value = audio[band];

    // finf peak of all bands
    if (value > pkLevel)
    {
      pkLevel = value;
      pkBand = band;
    }

    // find peak in this band
    if (value > peaks[band])
      peaks[band] = value;
    else
    {
      peaks[band] = peaks[band] - peakDecay;
      if (peaks[band] < 0)
        peaks[band] = 0;
    }
  }
}





// interpolate 7 bands into 16
void interpolate()
{
  float step = (1.0 * (EQ_BANDS7 - 1)) / (EQ_BANDS16 - 1);
  for (int i = 0; i < EQ_BANDS16; i++)
  {
    float v = i * step;
    int x = v;          // x = band
    v = v - x;          // percentage of next band

    if (i < EQ_BANDS16 - 1)
    {
      if (usePeaks)
        audio16[i] = (1.0 - v) * peaks[x] + v * peaks[x + 1];
      else
        audio16[i] = (1.0 - v) * audio[x] + v * audio[x + 1];
    }
    else
    {
      if (usePeaks)
        audio16[i] = peaks[x];
      else
        audio16[i] = audio[x];
    }
  }
}




void printAudioValues()
{
  // slow down output
  if (ticks % 20 == 0)
  {
    for (uint8_t band = 0; band < EQ_BANDS7; band++)
    {
      Serial.print(audio[band]);
      Serial.print("\t");
    }
    printValue();
    printValue("gain", gain);
    printValue("maxLevel", maxLevel);
    printValue("maxBand", maxBand);
    printValue("maxRaw", maxRaw);
    printValue("avgLevel", avgLevel);
    printValue();
  }
}
