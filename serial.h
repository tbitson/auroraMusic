/****************************************************
  serial.h - creates a serial interface to adjust
  many parameters of the display. Type '?' to list
  display.

  last update:  05Jul2021


*******************************************************/



#ifdef USE_SERIAL


// prototypes
void checkSerial();




void checkSerial()
{
  // check serial port for incomming command
  if (!Serial.available())
    return;

  // check for if its a valid char
  byte b = Serial.read();
  if (b < 32 || b > 126)
    return;

  // decode it
  switch (b)
  {

    case 'a':
      simAudio = !simAudio;
      Serial.print("simAudio ");
      simAudio ? Serial.println("Enabled") : Serial.println("Disabled");
      break;

    case 'd':
      printLevel++;
      if (printLevel > 3)
        printLevel = 0;
      printValue("printLevel", printLevel);
      break;

    case '+':
    case '=':
      pattern++;
      if (pattern > numPatterns)
        pattern = 0;
      printValue("pattern", pattern);
      eepromUpdate = true;
      break;

    case '-':
      if (pattern > 0)
        pattern--;
      printValue("pattern", pattern);
      eepromUpdate = true;
      break;

    case '1':
      if (pattern < numPatterns - 10)
        pattern += 10;
      eepromUpdate = true;
      break;

    case 's':
      delayVal++;
      printValue("delay", delayVal);
      eepromUpdate = true;
      break;

    case 'f':
      if (delayVal > 0)
        delayVal--;
      printValue("delay", delayVal);
      eepromUpdate = true;
      break;

    case 'c':
      colorMode++;
      if (colorMode > MAX_COLOR_MODES)
        colorMode = 0;
      printValue("colorMode", colorMode);
      break;

    case 'p':
      patternMode++;
      if (patternMode > MAX_PATTERN_MODES)
        patternMode = 0;
      printValue("patternMode", patternMode);
      break;

    case 'u':
      showFPS = !showFPS;
      Serial.print("showFPS ");
      showFPS ? Serial.println("Enabled") : Serial.println("Disabled");
      break;

    case 't':
      testMode = !testMode;
      Serial.print("testMode ");
      testMode ? Serial.println("Enabled") : Serial.println("Disabled");
      break;


    case 'T':
      audioDebug = !audioDebug;
      Serial.print("audioDebug ");
      audioDebug ? Serial.println("Enabled") : Serial.println("Disabled");
      break;

    case 'x':
      dmxDebug = !dmxDebug;
      Serial.print("dmxDebug ");
      dmxDebug ? Serial.println("Enabled") : Serial.println("Disabled");
      break;


    case '0':
      pattern = 0;
      eepromUpdate = true;
      break;

    case 'W':
      audioPatterns.white();
      break;


    case 'S':
      showSettings();
      break;


    case '?':
      Serial.println();
      Serial.print("Device name : "); Serial.println(deviceName);
      Serial.println("+/=) increment to next Pattern");
      Serial.println("-)  decrement to previous Pattern");
      Serial.println("0)  jump to pattern 0");
      Serial.println("1)  jump ahead 10 patterns");
      Serial.println("S)  show current Settings");
      Serial.println("a)  toggle Audio sim mode");
      Serial.println("f)  Faster display");
      Serial.println("s)  Slower display");
      Serial.println("c)  starBurst: cycle Color mode");
      Serial.println("p)  starBurst: cycle Pattern");
      Serial.println("u)  toggle print Update rate");
      Serial.println("t)  toggle audio testMode (raw audio values)");
      Serial.println("T)  toggle audio degug values");
      Serial.println("W)  display all White test pattern (use caution!)");
      Serial.println("x)  toggle DMX debug mode");
      Serial.println("d)  inc debug print level");
      Serial.println();
      break;

    default:
      Serial.println("huh?");
      break;
  }
}
#endif
