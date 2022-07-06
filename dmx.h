/********************************************
  dmx.h - DMX interface to allow DMX control of
  key functions

  last update: 15Jul2021

  coded to use SERIAL1 on digital pins 0 & 1
  uses 3 channels:
  0 - display on/off: 0 to 127 = off, > 128 = on
  1 - select pattern every 10 counts: 0 to numPatterns
  2 - options, so far experimental


  *** Needs cleanup / optimization ***


 ***********************************************/


#ifdef USE_DMX

#pragma once


#include <TeensyDMX.h>
namespace teensydmx = ::qindesign::teensydmx;

// number of DMX addresses to listen for
#define DMX_SIZE 3


// prototypes
void dmxInit();
void checkDMX();
void updateDMX();
void printDmxBuffer();



// delay between updates to minimize impact
#define DMX_DELAY   2000


// dmx address and number of channels to monitor
extern uint8_t dmxAddress;
const uint8_t numDmxChans = DMX_SIZE;


// globals
uint8_t buff[DMX_SIZE] = { 0 };
uint8_t dmxValues[DMX_SIZE] = { 0 };
uint32_t lastUpdate = 0;



// Create the DMX receiver on Serial1 (pins 1 & 2)
// requires an RS-485 convertor to interface to DMX bus
teensydmx::Receiver dmxRx{Serial1};



// initialize dms
void dmxInit()
{
  if (dmxAddress == 0)
    dmxAddress = DMX_ADDRESS;

  // configure the RS485 Transmit/Receive mode to receive
  pinMode(DMX_RXTX_PIN, OUTPUT);
  digitalWrite(DMX_RXTX_PIN, LOW);

  // Start the receiver
  dmxRx.begin();
  Serial.println("dmx started");

  // read and discard first dmx packet
  delay(1000);
  dmxRx.readPacket(buff, 0, DMX_SIZE);
  lastUpdate = millis();
}



// check for dmx commands targeted for this device
void checkDMX()
{
  if (lastUpdate - millis() > DMX_DELAY)
  {
    // just read data for this device
    int bytes = dmxRx.readPacket(buff, dmxAddress, numDmxChans);

    // did we get data?
    if (bytes == numDmxChans)
    {
      // did dmx packet change?
      if (memcmp(buff, dmxValues, numDmxChans) != 0)
      {
        if (memcpy(dmxValues, buff, numDmxChans) != 0)
        {
          // ok, now update display
          updateDMX();
        }
        else
          Serial.println("error copying buffer");
      }
    }
    else
    {
      if (dmxDebug)
      {
        if (bytes < 1)
          Serial.println("No DMX Data");
        else
        {
          Serial.print("bytes read =  ");
          Serial.println(bytes);
        }
      }
    }
  }
}


void updateDMX()
{
  printValue("dmx base addr", dmxAddress);
  printValue("addr 0", dmxValues[0]);
  printValue("addr 1", dmxValues[1]);
  printValue("addr 2", dmxValues[2]);


  // on/off control channel 0
  uint8_t onOff = dmxValues[0];
  if (onOff < 128)
  {
    // turn off
    pattern = 0;
    printValue("dmx pattern off", pattern);
  }
  else
    // select pattern
  {
    // increment pattern for every 10 dmx counts
    dmxValues[1] /= 10;
    if (dmxValues[1] < numPatterns)
    {
      pattern = dmxValues[1];
      printValue("pattern set", pattern);
    }
  }

  // experimental control stuff
  simAudio = false;

  if (dmxValues[2] < 32)
    rectColor =  WHITE;
  else if (dmxValues[2] < 64)
    rectColor =  RED;
  else if (dmxValues[2] < 96)
    rectColor =  GREEN;
  else if (dmxValues[2] < 128)
    rectColor =  BLUE;
  else if (dmxValues[2] < 160)
    rectColor =  PURPLE;
  else
    simAudio = true;
}

#endif
