/*******************************************************************
  AuroraMusic - LED Matrix Music Display
  Updated for SmartMatrix Vers. 4,  SmartLED Sheild V5 and Teensy 3.5, 3.6, 4.x.

  Kudos to Louis Beaudoin/Pixelmatix for developing ths awesome LED driver
  and always keeping the code neat & tidy. He must spend, i dunno, maybe 15 hours a day.

  Based on Aurora: https://github.com/pixelmatix/aurora  Copyright (c) 2014 Jason Coon
  Designed in collaboration with Louis Beaudoin/Pixelmatix using the awesome
  SmartMatrix Library: https://github.com/pixelmatix/SmartMatrix

  *** Key hardware options are define in hardware.h - be sure to check this file.


  Version History

  vers 3.0.0 - started with version 1.7 of aurora
  vers 3.1.0 - now saves current pattern to EEPROM, restores on next power up
               all patterns but radial circles are working
               added dmx.h, starting dmx interface
               serial commands updated/changed
  vers 3.2.0 - added dmxAddress, delayVal to EEPROM, optimized audioRead()
               added spiro pattern
  vers 3.3.x - added non-music pattern 'life'
               worked on DMX receive capability
               moved serial functions to separate file
  vers 3.4.x - cleanup, DMX bug fixes, added DMX delay
  vers 3.5.x - more cleanup, settings configured for 'release'
  vers 3.6.x - added #define SET_STARTUP_PATTERN in hardware.h to start
                  with a pre-define pattern for BHB
  vers 3.7.x - general cleanup, fixed StarBurst - now working
               added #define in hardware.h for rotation value
               moved kScreenWidth & kScreenHeight assignments after rotation call
               started fixing rotated patterns on non-square displays
               deleted some patterns that don't work, are redundant, or are just plain shitty
               removed all RTC code, added star color & pattern mode to eeprom
  vers 3.8.0   release candidate - fixed rotation bugs, cleanup
  vers 3.9.x   release for BHB as 4.0


  To Do:
      FIX use on bigLedFrame - out of memoery error due to addition of life pattern
      Add use of ext mem on Teensy 4.1
      Add GIF Player (pending switch to Teensy 4.1)
      Add new Default BHB Image

   lil music frame has ext mem:
   EXTMEM Memory Test, 8 Mbyte
   CCM_CBCMR=B5AE8304 (88.0 MHz)


  ports:
  lil music frame: 301 Teensy 411 - has 8mb extmem
  big music frame: 701 Teensy 4.1
  Stereo Cabinet : 901 Teensy 4.0 32x64



***********************************************************************/


#define VERSION "Aurora v394 30Jun2022"



// ------>  define the target display <-------

//#define BHB_DISPLAY1
//#define BHB_DISPLAY2
#define BIG_MUSIC_FRAME
//#define LIL_MUSIC_FRAME
//#define PANELS_32x64

//--------------------------------------------


// helpers
#define OFF       false
#define ON        true
#define DISABLED  false
#define ENABLED   true


// hardware.h must be included first, sets display specific settings
#include "hardware.h"

// global options in the following block
// see hardware.h for device specific options
// values marked with * are stored in EEPROM and re-loaded at startup
String deviceName  = DISPLAY_NAME;  // device name from hardware.h
uint8_t printLevel = 1;             // debug print level
bool simAudio      = DISABLED;      // enables simulated audio for testing
bool showFPS       = DISABLED;      // show update rate
bool testMode      = DISABLED;      // toggle raw audio in analyzer8 to check levels
bool dmxDebug      = DISABLED;      // enable dmx data prints
uint8_t dmxAddress = 0;             // * dmx channel
uint8_t pattern    = 1;             // * set default pattern
uint32_t delayVal  = DELAY_VAL;     // * overall display update rate
uint8_t persistance = 176;          // * how much to dim pixels stars


// global vars
bool eepromUpdate = false;          // flag to indicate eeprom data needs to be saved
uint8_t brightness = 255;           // display brightness
uint32_t ticks = 0;                 // just counts display updates


// global sceen size vars, will not be valid until after rotation is applied
uint16_t kScreenWidth;
uint16_t kScreenHeight;
uint16_t kMatrixCenterX;
uint16_t kMatrixCenterY;


// local includes
#include <MatrixHardware_Teensy4_ShieldV5.h>
#include <SmartMatrix4.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <PrintValues.h>



// prototypes for this file
void setup();
void loop();
void saveSettings();
void restoreSettings();
void showSettings();





// configure smartmatrix driver & allocate buffers, from hardware.h
#define COLOR_DEPTH 24
const uint16_t kMatrixWidth  = X_PANEL_SIZE * NUM_X_PANELS;
const uint16_t kMatrixHeight = Y_PANEL_SIZE * NUM_Y_PANELS;
const uint8_t kRefreshDepth = 24;
const uint8_t kDmaBufferRows = 4;
const uint8_t kPanelType = PANEL_TYPE;
const uint8_t kMatrixOptions = PANEL_OPTIONS;
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);


// rotation other than 0 is unpredictable at this time
rotationDegrees rotation = ROTATION;

// total leds for array allocations, rotation shouldn't affect this
const uint16_t kNumLEDs = kMatrixWidth * kMatrixHeight;


// nonvolatile eeprom struct for saving settings
EEPromData eepromData;


#include "readAudio.h"
#include "audioPatterns.h"
AudioPatterns audioPatterns;


#ifdef USE_SERIAL
#include "serial.h"
#endif


#ifdef USE_IR_REMOTE
#include "irRemote.h"
#endif

#ifdef USE_DMX
#include "dmx.h"
#endif




void setup()
{
  Serial.begin(57600);
  delay(3000);

  // init eeprom library
  EEPROM.begin();
  delay(100);

  // get saved parameters
  restoreSettings();

  // if flag is set, set default pattern on power-up
#ifdef SET_STARTUP_PATTERN
  pattern = DEFAULT_PATTERN;
#endif

  lastPattern = pattern;

  // start up matrix driver
  matrix.addLayer(&backgroundLayer);
  matrix.begin();

  matrix.setRotation(rotation);
  matrix.setBrightness(brightness);
  backgroundLayer.enableColorCorrection(true);


  // update global matrix size constants after rotation call
  kScreenWidth = matrix.getScreenWidth() - 1;
  kScreenHeight = matrix.getScreenHeight() - 1 ;
  kMatrixCenterX = kScreenWidth / 2;
  kMatrixCenterY = kScreenHeight / 2;

  // clear display by filling it black
  backgroundLayer.fillScreen(BLACK);

  // send led display buffer to matrix - this is what actually updates the leds
  backgroundLayer.swapBuffers();

  // init patterns
  audioPatterns.init();

  Serial.println(VERSION);

  // setup effects functions
  effects.setup();
  Serial.println("effects initialized");

  // init MSGEQ7 audio chip
  initAudio();
  Serial.println("audio initialized");

  // init IR Remote
#ifdef USE_IR_REMOTE
  initIRRemote();
  Serial.println("ir remoted init");
#endif

  // configure serial interface
#ifdef USE_SERIAL
  Serial.println("serial enabled");
#endif

  // set up dmx if used
#ifdef USE_DMX
  dmxInit();
  Serial.println("dmx enabled");
#endif

  Serial.println("setup complete");
  Serial.println();
}




void loop()
{
#ifdef USE_SERIAL
  // check serial port for data & handle
  checkSerial();
#endif

#ifdef USE_DMX
  // check for dmx change
  checkDMX();
#endif

  // update audio pattern
  audioPatterns.update();

  // print the frames per second for debug
  if (showFPS) matrix.countFPS();

#ifdef USE_IR_REMOTE
  // check for pending ir commands
  checkIRRemote();
#endif


  // slow things down a bit
  delay(delayVal);

  // periodically check if eeprom data needs to be saved
  if (ticks % 5000 < 5)
  {
    if (eepromUpdate)
      saveSettings();
  }

  // increment loop counter
  ticks += 1;
}



void saveSettings()
{
  eepromData.vers = EEPROM_VERS;
  eepromData.dmxAddress = dmxAddress;
  eepromData.pattern = pattern;
  eepromData.delayVal = delayVal;
  eepromData.starPattern = patternMode;
  eepromData.starColor = colorMode;
  eepromData.persistance = persistance;
  eepromData.checkValue = EEPROM_CHECK_VALUE;

  if (printLevel > 0) Serial.println("updating eeprom data");
  EEPROM.put(EEPROM_START_ADDR, eepromData);
  eepromUpdate = false;
}




// clear eeprom contents (dev use only so far)
void clearEEPROM()
{
  Serial.println("Clearing EEPROM...");
  printValue("EEPROM size = ", EEPROM.length());

  for (int i = 0 ; i < EEPROM.length() ; i++)
    EEPROM.write(i, 0xFF);

  Serial.println("done");
}



void restoreSettings()
{
  EEPROM.get(EEPROM_START_ADDR, eepromData);
  if (printLevel > 0)
  {
    printValue("eeprom vers", eepromData.vers);
    printHexValue("eeprom checkValue", eepromData.checkValue);
  }

  if (eepromData.vers != EEPROM_VERS || eepromData.checkValue != EEPROM_CHECK_VALUE)
  {
    clearEEPROM();
    eepromData.vers = EEPROM_VERS;
    eepromData.checkValue = EEPROM_CHECK_VALUE;
    saveSettings();
    Serial.println("eeprom reset");
  }
  else
  {
    dmxAddress = eepromData.dmxAddress;
    pattern = eepromData.pattern;
    delayVal = eepromData.delayVal;
    patternMode = eepromData.starPattern;
    colorMode = eepromData.starColor;
    persistance = eepromData.persistance;
  }
  eepromUpdate = false;
}





void showSettings()
{
  Serial.println();
  Serial.println(VERSION);
  Serial.print("device name  : "); Serial.println(deviceName);
  Serial.print("file name    : "); Serial.println( __FILE__);
  Serial.print("date         : "); Serial.println( __DATE__);
  Serial.print("platform     : "); Serial.println(PLATFORM);
  Serial.print("matrix width : "); Serial.println(kMatrixWidth);
  Serial.print("matrix height: "); Serial.println(kMatrixHeight);
  Serial.print("rotation     : "); Serial.println(rotation);
  Serial.print("screen width : "); Serial.println(kScreenWidth);
  Serial.print("screen hieght: "); Serial.println(kScreenHeight);
  Serial.print("center X     : "); Serial.println(kMatrixCenterX);
  Serial.print("center Y     : "); Serial.println(kMatrixCenterY);
  Serial.print("audio gain   : "); Serial.println(gain);
  Serial.print("simAudio     : "); simAudio ? Serial.println("Enabled") : Serial.println("Disabled");
  Serial.print("testMode     : "); testMode ? Serial.println("Enabled") : Serial.println("Disabled");
  Serial.print("delayVal     : "); Serial.println(delayVal);
  Serial.print("printLevel   : "); Serial.println(printLevel);
  Serial.print("num Patterns : "); Serial.println(numPatterns);
  Serial.print("curr Pattern : "); Serial.print(pattern); Serial.print(" = "); Serial.println(patternName[pattern]);
  Serial.print("star pattern : "); Serial.println(patternMode);
  Serial.print("star color   : "); Serial.println(colorMode);

#ifdef USE_SERIAL
  Serial.println("Serial       : Enabled");
#else
  Serial.println("Serial       : Disabled");
#endif


#ifdef USE_DMX
  Serial.println("DMX         : Enabled");
  Serial.print("Num DMX Addr: ");
  Serial.println(DMX_SIZE);
  Serial.print("DMX Address : ");
  Serial.println(DMX_ADDRESS);
#else
  Serial.println("DMX         : Disabled");
#endif


#ifdef USE_IR_REMOTE
  Serial.println("IR Remote   : Enabled");
#else
  Serial.println("IR Remote   : Disabled");
#endif


#ifdef BOUNDS_CHECKING
  Serial.println("bounds check: Enabled");
#endif

  Serial.println();
}
