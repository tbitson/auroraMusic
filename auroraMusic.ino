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
  vers 3.9.x   cleanup for release
  vers 4.0.0.  release version, split off BHB version 31Jul2022
  vers 4.0.3   added pattern autoincrement mode, increments pattern every 60 seconds. 
               Removed all rotate options



  To Do:
      FIX use on bigLedFrame - out of memoery error due to addition of life pattern
      Add use of ext mem on Teensy 4.1
      Update to new IRremote & FastLED 3.4



**********************************************************************/


#define VERSION "Aurora v403 17Jan2023"



// helpers
#define OFF       false
#define ON        true
#define DISABLED  false
#define ENABLED   true


// hardware.h must be included first, sets display specific settings
#include "hardware.h"



// ---- eeprom config ----------------

#define EEPROM_START_ADDR   0x00
#define EEPROM_VERS         41
#define EEPROM_CHECK_VALUE  0xA5


struct EEPromData {
  uint8_t vers;
  uint8_t pattern;
  uint8_t starPattern;
  uint8_t starColor;
  uint8_t persistance;
  uint8_t checkValue;
  uint16_t dmxAddress;
  uint32_t delayVal;
};




// global options
// see hardware.h for device specific options
// values marked with * are stored in EEPROM and re-loaded at startup
bool simAudio       = DISABLED;      // enables simulated audio for testing
bool showFPS        = DISABLED;      // show update rate
bool testMode       = DISABLED;      // toggle raw audio in analyzer8 to check levels
bool dmxDebug       = DISABLED;      // enable dmx data prints
bool autoincrement  = false;         // auto-increment the pattern
String deviceName   = DISPLAY_NAME;  // device name from hardware.h
uint8_t printLevel  = 1;             // debug print leveluint8_t 
uint8_t pattern     = 1;             // * set starting pattern
uint8_t persistance = 176;           // * how much to dim pixels stars
uint16_t dmxAddress = DMX_ADDRESS;   // * dmx address
uint32_t delayVal   = DELAY_VAL;     // * overall display update rate


// global vars
bool eepromUpdate   = false;         // flag to indicate eeprom data needs to be saved
uint8_t brightness  = 255;           // display brightness
uint32_t ticks      = 0;             // just counts display updates
uint32_t lastSwitch = 0;             // time of last auto pattern switch


// global screen size vars, will not be valid until after rotation is applied
uint16_t kScreenWidth;
uint16_t kScreenHeight;
uint16_t kMatrixCenterX;
uint16_t kMatrixCenterY;


// local includes
#include <FastLED.h>
#include <MatrixHardware_Teensy4_ShieldV5.h>
#include <SmartMatrix4.h>
#include <EEPROM.h>
#include <PrintValues.h>


// prototypes for this file
void setup();
void loop();
void saveSettings();
void restoreSettings();
void clearEEPROM();
void showSettings();

// nonvolatile eeprom struct for saving settings
EEPromData eepromData;





// configure smartmatrix panel settings & allocate buffers, from hardware.h
// these comstants are idendified by 'k'
#define COLOR_DEPTH 24
const uint16_t kMatrixWidth  = X_PANEL_SIZE * NUM_X_PANELS;
const uint16_t kMatrixHeight = Y_PANEL_SIZE * NUM_Y_PANELS;
const uint8_t  kRefreshDepth = 24;
const uint8_t  kDmaBufferRows = 4;
const uint8_t  kPanelType = PANEL_TYPE;
const uint8_t  kMatrixOptions = PANEL_OPTIONS;
const uint8_t  kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

// total leds for array allocations, rotation shouldn't affect this
const uint32_t kNumLEDs = kMatrixWidth * kMatrixHeight;


// include modules
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

  Serial.println(VERSION);

  // init eeprom library
  EEPROM.begin();
  delay(100);

  // get saved parameters
  restoreSettings();

  //  pattern = defaultPattern;
  lastPattern = pattern;

  // start up matrix driver
  matrix.addLayer(&backgroundLayer);
  matrix.begin();

  matrix.setRotation(rotation0);
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
  // check serial port for data & handle
#ifdef USE_SERIAL
  checkSerial();
#endif

  // check for dmx change
#ifdef USE_DMX
  checkDMX();
#endif

  // update audio pattern
  audioPatterns.update();

  // print the frames per second for debug
  if (showFPS) matrix.countFPS();

  // check for pending ir commands
#ifdef USE_IR_REMOTE
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



// clear eeprom contents (dev use only so far)
void clearEEPROM()
{
  Serial.println("Clearing EEPROM...");
  printValue("EEPROM size = ", EEPROM.length());

  for (int i = 0 ; i < EEPROM.length() ; i++)
    EEPROM.write(i, 0xFF);

  Serial.println("done");
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
  Serial.println("DMX          : Enabled");
  Serial.print("Num DMX Addr: ");
  Serial.println(DMX_SIZE);
  Serial.print("DMX Address : ");
  Serial.println(DMX_ADDRESS);
#else
  Serial.println("DMX          : Disabled");
#endif

#ifdef USE_IR_REMOTE
  Serial.println("IR Remote    : Enabled");
#else
  Serial.println("IR Remote    : Disabled");
#endif

#ifdef BOUNDS_CHECKING
  Serial.println("bounds check: Enabled");
#endif

  Serial.println();
}
