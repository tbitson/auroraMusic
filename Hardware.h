/*****************************************************************
  hardware.h - Primary configurations and hardware connections

  vers. 3.0    Feb2022

    NOTE! This is written to support various display configs by defining the
    target display below:
    - Teensy Version
    - Matrix size: panel size & number of panels (assumes square 2x2, 3x3, 4x4, etc)
    - SmartMatrix Adaptor: NOTE the older Vers 4 sheild for Teensy 3.x (because I had them)
         and newer version 5 sheild for Teensy 4.x (because they're required)
    - MSEQ7 Connections
    - IR Remote connections if used
    - DMX configuration
    - Test LED points if used
    - limit on Teensy 4.1 seems to be ~ 41820 pixels

    Use this as a guide for your configuration, but many options are available,
    and delete the unused defines for clarity. I tried to be consistent with
    the larger form factor Teensy.


    SmartMatrix dedicated DIO for SmartLED V4 & teensy 3.x
    ----------------------------------------------------
    2 - Buffer_R1        23 -
    3 - Buffer_Latch     22 -
    4 - Buffer_OE        21 - Buffer_R2
    5 - Buffer_G1        20 - Buffer_B2
    6 - Buffer_B1        19 -
    7 - SPI_MOSI*        18 -
    8 - Buffer_G2        17 - LED_EN*
    9 -                  16 -
    10-                  15 -
    11-                  14 - Buffer_CLK
    12-                  13 - SPI_CLK*


    SmartMatrix dedicated DIO for SmartLED Sheild V5 with Teensy 4.0/4.1
    ----------------------------------------------------
    2 - Buffer_OE       23 -
    3 - Buffer_Latch    22 - LED_EN*
    4 - FLEXIO_SCK*     21 -
    5 - FLEXIO_MOSI*    20 -
    6 - Buffer_R1       19 -
    7 - Buffer_CLK      18 -
    8 - Buffer_CLK Alt  17 -
    9 - Buffer_G1       16 -
    10- Buffer_B1       15 -
    11- Buffer_G2/MOSI  14 -
    12- Buffer_R2       13 - Buffer_B2/SCK

       used by APA102 LED Strip driver only, can be shared

******************************************************************/


#pragma once


// ------>  define the target display <-------

#define BIG_MUSIC_FRAME
//#define LIL_MUSIC_FRAME
//#define STEREO_CABINET
//#define XLIGHTS



// --------------------------------------------


// teensy 3.2 might work on a single 64x64 panel but I didnt try it
#ifdef ARDUINO_TEENSY32
#define PLATFORM "Teensy_32"
#pragma GCC error "This version not compatible with Teensy 3.2"
#elif defined ARDUINO_TEENSY35
#define PLATFORM "Teensy_35"
#elif defined ARDUINO_TEENSY36
#define PLATFORM "Teensy_36"
#elif defined ARDUINO_TEENSY40
#define PLATFORM "Teensy_40"
#elif defined ARDUINO_TEENSY41
#define PLATFORM "Teensy_41"
#else
#pragma GCC error "No valid Teensy ID Found - Select TEENSY from Tools -> Boards"
#endif



// --- dev use - turn off in release ----
//#define BOUNDS_CHECKING

// --- display smartMartrix compiler messages ---
//#define SM_SHOW_MESSAGES

#define AUTO_SWITCH_DURATION 60000



// settings for the specific displays
//---------------------------------------------------

#ifdef BIG_MUSIC_FRAME
#define DISPLAY_NAME        "Big Music Frame"

// 196 x 196 pixels
#define NUM_X_PANELS        3
#define NUM_Y_PANELS        3
#define X_PANEL_SIZE        64
#define Y_PANEL_SIZE        64
#define PANEL_TYPE          SMARTMATRIX_HUB75_64ROW_MOD32SCAN
#define PANEL_OPTIONS       SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING | SMARTMATRIX_OPTIONS_C_SHAPE_STACKING
#define COLOR_MODE_BRG

#define MSGEQ7_AUDIO_PIN    A17
#define MSGEQ7_STROBE_PIN   40
#define MSGEQ7_RESET_PIN    39
#define BACKGROUND_OFFSET   {60, 60, 60, 60, 60, 65, 70}

#define DELAY_VAL           5

// setting for this display
#define USE_SERIAL
//#define INCLUDE_LIFE
//#define USE_IR_REMOTE
//#define USE_DMX

#define IR_RECV_GND_PIN     32
#define IR_RECV_POWER_PIN   31
#define IR_RECV_DATA_PIN    30

#define DMX_ADDRESS       60
#define DMX_TX_PIN        0
#define DMX_RX_PIN        1
#define DMX_RXTX_PIN      2



//---------------------------------------------------

#elif defined LIL_MUSIC_FRAME
#define DISPLAY_NAME  "Lil Music Frame"

// 128x 128 pixels
#define NUM_X_PANELS        1
#define NUM_Y_PANELS        2
#define X_PANEL_SIZE        128
#define Y_PANEL_SIZE        64
#define PANEL_TYPE          SMARTMATRIX_HUB75_64ROW_MOD32SCAN;
#define PANEL_OPTIONS       SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING | SMARTMATRIX_OPTIONS_C_SHAPE_STACKING
#define COLOR_MODE_RBG

// setting for this display
#define USE_SERIAL
#define INCLUDE_LIFE
//#define USE_IR_REMOTE
//#define USE_DMX

#define DELAY_VAL           5

#define MSGEQ7_AUDIO_PIN    A5
#define MSGEQ7_STROBE_PIN   21
#define MSGEQ7_RESET_PIN    20
#define BACKGROUND_OFFSET   {60, 80, 70, 100, 90, 120, 140}

#define IR_RECV_GND_PIN     18
#define IR_RECV_POWER_PIN   17
#define IR_RECV_DATA_PIN    16

#define DMX_ADDRESS         60
#define DMX_TX_PIN          0
#define DMX_RX_PIN          1
#define DMX_RXTX_PIN        2


//--------------------------------------------------------------

// 256 x 256 pixels - won't compile on T4.1 due to memory overflow
// 64 x 128 P2 Modules arranged 2w x 4h
// panel = 256 x 256 P2 leds in linear rows
// panels dont seem to be standard hub75 compatible

#elif defined XLIGHTS
#define DISPLAY_NAME        "xlights"

#define NUM_X_PANELS        2
#define NUM_Y_PANELS        4
#define X_PANEL_SIZE        128
#define Y_PANEL_SIZE        64
#define PANEL_TYPE          SMARTMATRIX_HUB75_64ROW_MOD32SCAN  //SMARTMATRIX_HUB75_32ROW_MOD16SCAN
#define PANEL_OPTIONS       SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING
#define COLOR_MODE_RBG

// setting for this display
#define USE_SERIAL
#define INCLUDE_LIFE
//#define USE_IR_REMOTE
//#define USE_DMX

#define MSGEQ7_AUDIO_PIN    A9
#define MSGEQ7_STROBE_PIN   22
#define MSGEQ7_RESET_PIN    21
#define BACKGROUND_OFFSET   {60, 55, 50, 50, 56, 65, 70}

#define ROTATION            rotation0
#define DELAY_VAL           5

#define IR_RECV_GND_PIN     18
#define IR_RECV_POWER_PIN   17
#define IR_RECV_DATA_PIN    16


#define DMX_ADDRESS         60
#define DMX_TX_PIN          0
#define DMX_RX_PIN          1
#define DMX_RXTX_PIN        2

//--------------------------------------------------------------

#elif defined STEREO_CABINET
#define DISPLAY_NAME        "Stereo Cabinet"

// panels rotated, so x & y are swapped
// 64 x 160 becomes 160x64
// 192 x 64 pixels

#define NUM_X_PANELS        1
#define NUM_Y_PANELS        5
#define X_PANEL_SIZE        64
#define Y_PANEL_SIZE        32
#define PANEL_TYPE          SMARTMATRIX_HUB75_32ROW_MOD16SCAN
#define PANEL_OPTIONS       SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING
#define COLOR_MODE_RBG

// setting for this display
#define USE_SERIAL
#define INCLUDE_LIFE
//#define USE_IR_REMOTE
//#define USE_DMX


#define MSGEQ7_AUDIO_PIN    A9
#define MSGEQ7_STROBE_PIN   22
#define MSGEQ7_RESET_PIN    21
#define BACKGROUND_OFFSET   {60, 55, 50, 50, 56, 65, 70}

#define DELAY_VAL           5

#define IR_RECV_GND_PIN     18
#define IR_RECV_POWER_PIN   17
#define IR_RECV_DATA_PIN    16

#define DMX_ADDRESS         60
#define DMX_TX_PIN          0
#define DMX_RX_PIN          1
#define DMX_RXTX_PIN        2



//---------------------------------------------------

#else
#pragma GCC error "ERROR - Target display not defined. Update AuroraMusic.ino"
#endif
