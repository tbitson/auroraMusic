AuroraMusic Version 3.9.4

AuroraMusic was started with Louis Beaudoin/Pixelmatix's Aurora LED Matrix display. I really enjoyed the Music display options and started creating variations on existing display and incorporating new designs. As it evolved, memory constraints dictated removal of the non-music patterns. 

Version 3.x was ported to the latest Pixelmatix Smartmatrix 4.0 library and v5 of the SmartLED interface. It requires a Teensy 3.5, 3.6, 4.0, or 4.1. 



General notes:
There are 2 non-music displays: rects and life. rects is just a series of rectangles (squares actually) drawn on the display primarily for testing. Life is based on the classic Life simulation/algorithm simply because i really like it.

The configuration of you specific hardware setup is done in hardware.h.

As with the original, pattern selection can be done with a simple IR remote. Specifically, a SparkFun IR remote. If you want to use a different brand, you'll have to update the message configuration in irRemote.h

Interaction with the display can also be done using a serial interface to the Teensy USB port. When using Arduino IDE, simply open the serial window and type a '?' to see the options.


Hardware:
Teensy 3.5, 3.6, 4.0, or 4.1. 

At least 1 LED Matrix Panel of 32x32 pixels with a HUB75 interface. A teensy 4.1 with support up to 192x192 pixel arraingment. See details at the Smartmatrix library github page.

A SmartLED (formally Smartmatrix) interface V4 or V5 to interface the teensy to the led matrix.

A MEGEQ07 based audio interface. This converts incoming audio into 7 'equalizer' bands for the teensy to read. There are many breakout boards available such as SparkFun, adafruit, etc.

Optional: IR Remote receiver. This is a 3-pin device that reads and demodulates 38KHz IR signals. 




