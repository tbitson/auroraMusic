AuroraMusic - LED Matrix Music Display

Displays visual audio patterns on common LED matrix panels that
support the HUB75/HUB75E interface. Supports up to 192x192 pixel
displays (maybe higher...)

Inspiration came from the original Aurora Display from Louis Beaudoin and
Jason Coon @ Pixelmatix. Simply awesome! I prefered the audio patterns, so I
stripped out the non-audio stuff to make room for more audio patterns. And
then Paul @ PRJC created Teensy 3.x & 4.x with more memory and CPU power...

Kudos to Louis Beaudoin/Pixelmatix for developing this awesome LED driver
and always keeping the code neat & tidy. He must spend, i dunno, maybe 15 hours a day.

Some audio patterns based on Aurora: https://github.com/pixelmatix/aurora, 
Copyright (c) 2014 Jason Coon. Designed in collaboration with Louis Beaudoin/Pixelmatix
using the awesome SmartMatrix Library: https://github.com/pixelmatix/SmartMatrix

Updated for SmartMatrix Vers. 4,  SmartLED Sheild V5 and Teensy 3.5, 3.6, 4.x.

DMX support was added so the Brad Harris Band could use these on stage.

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
               deleted some patterns that don't work, are redundant, or are just
               plain shitty. removed all RTC code, added star color & pattern mode to eeprom
  vers 3.8.0   release candidate - fixed rotation bugs, cleanup
  vers 3.9.x   cleanup for release
  vers 4.0.0.  release version, split off BHB version 31Jul2022
  vers 4.0.3   added pattern autoincrement mode, increments pattern every 60 seconds.
               Removed all rotate options since it madesome patterns wonky.



  To Do:
      FIX use on bigLedFrame - out of memoery error due to addition of life pattern
      Add use of ext mem on Teensy 4.1
      Update to new IRremote & FastLED 3.4


