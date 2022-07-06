/*******************************************************

   Star.h - star class - creates and moves the stars

  This is the star class, and typically 200-400 stars are
  instaniated in an array. A star may be tagged as 'alive'
  or not. Dead stars are simply black and do not move.
  Based on the audio input, the next "available" dead star is
  assigned a color, speed/direction, speed decay, brightness,
  brightness decay, and is marked 'alive'. Functions in the
  class then move the star across the display, decrementing
  its brightness and speed until it reaches a lower threshold
  where it "dies" and get recycled ("rebirth").

  This variation creates stars randomly in x-axis, with a CCW
  direction. If star hits lower edge, it is transformed onto
  the bottom panel with same speed & heading, but due to the
  45 degree rotation of the bottom panel, the tranformation
  is angled. Maybe I'll fix that someday. If the stars on bottom
  panel hit an edge, they die (for now).Stars also have long
  'tails'. This started in dev to follow movements, and I kinda
  liked it. Adjust using the persistance setting.


 *********************************************************/

#pragma once



#include "colorUtils.h"

// dev star print tool
//#define PRINT_ENABLE


// constants for starburst
const uint16_t NUM_STARS = 300;       // number of total 'stars'
uint8_t MAX_COLOR_MODES = 4;          // number of available color modes
uint8_t MAX_PATTERN_MODES = 3;        // number of available color modes


// prototypes - non-class functions
void incrementAngle();


// settings constants/globals
int8_t patternMode   = 0;             // curent pattern selection
int8_t colorMode     = 0;             // color generation mode - see star class

uint8_t currentColor = 0;
float angle          = 0;
uint8_t wheelPos     = 0;

const uint8_t MIN_BRIGHTNESS = 5;
const float   MIN_SPEED      = 0.08;




class Star
{
  public:
    Star();
    bool starBirth();
    void tick();
    bool isAlive();

  private:
    void setPattern(int8_t);
    void setColor(int8_t);
    void drawStar();
    void eraseStar();
    void bounceOffWalls();
    void printStar();

    bool _alive;
    float _x;
    float _y;
    rgb24 _color;
    float _brightnessDecay;
    float _xSpeed;
    float _ySpeed;
    float _speedDecay;
};


// quickie function to return absolute value of float
float pos(float num)
{
  return (num > 0) ? num : -num;
}



// instantiate new empty star
Star :: Star()
{
  _alive = true;

  // start in center
  _x = float(kMatrixCenterX);
  _y = float(kMatrixCenterY);
}



bool Star :: starBirth()
{
  // create new star if power on and audio exceeds threshold
  if (maxLevel > lowThreshold)
  {
    // start in center
    _x = float(kMatrixCenterX);
    _y = float(kMatrixCenterY);

    // configure star charactoristics
    setColor(colorMode);
    setPattern(patternMode);

    // increment angle for next star
    incrementAngle();

    _alive = true;
  }

  else
  {
    _color = BLACK;
    _alive = false;
  }

  return _alive;
}



void Star :: setColor(int8_t mode)
{

  switch (mode)
  {
    // basic colors based on eq level (default mode)
    case 0:
      _color = rgb24Colors8[maxBand];
      break;

    // cycle through color wheel
    case 1:
      wheelPos++;
      if (wheelPos > 254) wheelPos = 0;
      _color = wheel8(wheelPos);
      break;

    // assign color based on ticks
    case 2:
      _color = wheel8(ticks % 256);
      break;

    // color based on distance from center
    case 3:
      // color updated in tick() function
      _color = rgb24Colors8[0];
      break;

    default:
      colorMode = 0;
      Serial.println("Error: Invalid color mode");
  }
}



void Star :: setPattern(int8_t mode)
{
  float speed;

  switch (mode)
  {
    case 0:
      // increase speed with volume
      speed = 0.50 + (avgLevel / 300);
      speed = constrain(speed, 0.25, 2.0);
      _xSpeed = cos(angle) * speed;
      _ySpeed = sin(angle) * speed;

      // speed decay factor
      _speedDecay = (float)random(980, 990) / 1000.0;

      // brightness decay value. 1.00 is no decay
      _brightnessDecay = (float)random(980, 999) / 1000.0;

      persistance = 220;
      break;


    // long tails
    case 1:
      // increase speed with volume
      speed = 0.50 + (avgLevel / 300);
      speed = constrain(speed, 0.25, 2.0);

      // calc x & y speeds
      _xSpeed = cos(angle) * speed;
      _ySpeed = sin(angle) * speed;

      // speed decay factor
      _speedDecay = 0.990; // nominal decay value

      // brightness decay value
      _brightnessDecay = (float)random(940, 999) / 1000.0;

      persistance = 240;
      break;

    case 2:
      // fixed settings
      speed = 0.99;
      _xSpeed = cos(angle) * speed;
      _ySpeed = sin(angle) * speed;
      _speedDecay = 0.990;
      _brightnessDecay = 0.998;
      persistance = 220;
      break;

    default:
      patternMode = 0;
      Serial.println("Error: Invalid pattern mode");
  }
}




// increment star position
void Star :: tick()
{
  // old method to erase star if dimming is not used
  // eraseStar();

  // color mode 4 must be updated while star is moving
  if (colorMode == 4)
  {
    // calc distance from center
    float x = pos(_x - kMatrixCenterX);
    float y = pos(_y - kMatrixCenterY);
    float dist = sqrt((x * x) + (y * y));
    // scale for best looking color range
    dist = constrain(dist * (180 / kMatrixCenterX), 0, 255);
    _color = wheel8Sat(((uint8_t)dist & 255), 84);
  }

  // increment x & y positions
  _x += _xSpeed;
  _y += _ySpeed;

  // hit wall?
  bounceOffWalls();


  // decrease speed each tick
  _xSpeed *= _speedDecay;
  _ySpeed *= _speedDecay;

  // check speed and die if too slow
  if (abs(_xSpeed) < MIN_SPEED && abs(_ySpeed) < MIN_SPEED)
  {
    _alive = false;
    return;
  }

  // decrease star brightness each tick
  _color.red   *= _brightnessDecay;
  _color.green *= _brightnessDecay;
  _color.blue  *= _brightnessDecay;

  // check brightness and die if too dim
  if (_color.red < MIN_BRIGHTNESS && _color.green < MIN_BRIGHTNESS && _color.blue < MIN_BRIGHTNESS)
  {
    _alive = false;
    return;
  }

  // now draw updated star
  drawStar();
}



void Star :: eraseStar()
{
  backgroundLayer.drawPixel((uint16_t)_x, (uint16_t)_y, BLACK);
}



void Star :: drawStar()
{
  // don't need to update dead (black) stars
  if (!_alive)
    return;

  // draw the pixel (a.k.a. star)
  backgroundLayer.drawPixel(uint16_t(_x), uint16_t(_y), _color);

}



bool Star :: isAlive()
{
  return _alive;
}



void incrementAngle()
{
  // increment global var angle
  angle += 1.2; //(float)random(10, 200) / 100;
  if (angle >= TWO_PI) angle -= TWO_PI;
}



void Star :: bounceOffWalls()
{
  // bounce off walls
  if (_x > kScreenWidth - 1)
  {
    _x = kScreenWidth - 1;
    _xSpeed *= -1.0;
  }

  if (_x < 0)
  {
    _x = 0;
    _xSpeed *= -1.0;
  }

  if (_y > kScreenHeight - 1)
  {
    _y = kScreenHeight - 1;
    _ySpeed *= -1.0;
  }

  if ( _y < 0)
  {
    _y = 0;
    _ySpeed *= -1.0;
  }
}



#ifdef PRINT_ENABLE
void Star :: printStar()
{
  if (_alive && ticks % 100)
  {
    Serial.print("x: "); Serial.print(_x);
    Serial.print("    y: "); Serial.println(_y);

    Serial.print("xSpeed:   "); Serial.print(_xSpeed);
    Serial.print("   ySpeed:   "); Serial.println(_ySpeed);
    Serial.print("spdDecay: "); Serial.println(_speedDecay, 4);

    Serial.print("red:   0x"); Serial.println(_color.red, HEX);
    Serial.print("green: 0x"); Serial.println(_color.green, HEX);
    Serial.print("blue:  0x"); Serial.println(_color.blue, HEX);

    Serial.print("brightDecay: "); Serial.println(_brightnessDecay);
    Serial.println();
  }
}
#endif
