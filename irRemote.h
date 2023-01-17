/***********************************************
    irRemote.h - checks for IR remote commands
    and decodes results

    Vers. 1.2 April 2021

    Derived from the great irRemote library by
    Ken Sherrif.

    Requires Vers 2.3 of IRremote
    

    *** This code specifically tailored for a Sparkfun
    IR Remote. Change the codes below to match your
    remote if it isn't the Sparkfun one. ***

**************************************************/


#ifdef USE_IR_REMOTE

#pragma once

#include <IRremote.h>



// prototypes
void initIRRemote();
void checkIRRemote();
void processCode(uint32_t code);



#ifdef USE_IR_REMOTE
IRrecv irReceiver(IR_RECV_DATA_PIN);
#endif

// global data
decode_results results;
uint32_t irCode;
uint32_t lastCode;


extern const uint8_t numPatterns;


// IR Raw Key Codes for SparkFun remote
#define IRCODE_HELD         0xFFFFFFFF
#define IRCODE_POWER        0x10EFD827
#define IRCODE_A            0x10EFF807
#define IRCODE_B            0x10EF7887
#define IRCODE_C            0x10EF58A7
#define IRCODE_UP           0x10EFA05F
#define IRCODE_DOWN         0x10EF00FF
#define IRCODE_LEFT         0x10EF10EF
#define IRCODE_RIGHT        0x10EF807F
#define IRCODE_CENTER       0x10EF20DF

#define DISABLE_LED_FEEDBACK_FOR_RECEIVE




void initIRRemote()
{
  // enable power if using I/O pins for power
#ifdef IR_RECV_POWER_PIN
  pinMode(IR_RECV_GND_PIN, OUTPUT);
  pinMode(IR_RECV_POWER_PIN, OUTPUT);
  digitalWrite(IR_RECV_GND_PIN, LOW);
  digitalWrite(IR_RECV_POWER_PIN, HIGH);
  delay(1000);
#endif

  // Initialize the IR receiver
  //IrReceiver.begin(IR_RECV_DATA_PIN);
    irReceiver.enableIRIn();
  delay(200);

}



void checkIRRemote()
{
  // is an ir cmd avalable?
  if (irReceiver.decode(&results))
  {
    irCode = results.value;
    //Serial.print("IR code = 0x"); Serial.println(irCode, HEX);

    if (irCode != IRCODE_HELD)
    {
      lastCode = irCode;
      processCode(irCode);
    }
    else if (irCode == IRCODE_HELD)
      processCode(lastCode);

    // clear code and listen for new one
    irReceiver.resume();
  }
}




void processCode(uint32_t code)
{
  switch (code)
  {
    case IRCODE_POWER:
      if (brightness == 255)
        brightness = 0;
      else
        brightness = 255;
      break;

    case IRCODE_CENTER:
      Serial.println("Restore Default Settings");
      pattern = 0;
      delayVal = 25;
      simAudio = false;
      brightness = 255;
      printLevel = 0;
      testMode = OFF;
      eepromUpdate = true;
      lastCode = 0;  // prevents repeated sends
      break;

    case IRCODE_UP:
      Serial.println("Faster");
      if (delayVal >= 0)
        delayVal--;
      printValue("delayVal", delayVal);
      eepromUpdate = true;
      break;

    case IRCODE_DOWN:
      Serial.println("Slower");
      delayVal++;
      printValue("delayVal", delayVal);
      eepromUpdate = true;
      break;

    case IRCODE_LEFT:
      Serial.println("Previous Pattern");
      if (pattern > 0)
        pattern--;
      break;

    case IRCODE_RIGHT:
      Serial.println("Next Pattern");
      if (pattern < numPatterns)
        pattern++;
      break;


    case IRCODE_A:
      Serial.println("IRCODE_A");
      break;

    case IRCODE_B:
      Serial.println("IRCODE_B");
      break;

    case IRCODE_C:
      Serial.println("IRCODE_C");
      break;
  }
}

#endif
