/*******************************************************************
  life.h - an adaption of Conway's 'Game of Life'
  This is a "non-audio" pattern that just runs on its own

  version 2.13

  Adapted from the Life example on the Processing.org site

    Game of life rules:
    Black = dead, color = alive, color changes as cell ages
    Each iteration = one generation
  1. Any live cell with two or three live neighbours lives on to the next generation.
  2. Any live cell with fewer than two live neighbours dies, as if caused by underpopulation
  3. Any live cell with more than three live neighbours dies, as if by overpopulation
  4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.

 *********************************************************************/

#pragma once


#include "colorUtils.h"
#include <PrintValues.h>

const uint8_t startingColor = 85;   // green
const uint8_t deadColor = 0x00;    // black


class Cell {
  public:
    bool    alive;
    bool    prev;
    uint8_t color;
};



class Life {
  public:
    Life();
    void updateWorld();


  private:
    Cell cells[kMatrixWidth][kMatrixHeight];

    uint16_t generation = 0;
    uint16_t maxGenerations = 8000;
    uint16_t newCells = 0;
    uint16_t lastCount = 0;
    uint16_t endCounter = 0;

    void createWorld();
    uint16_t countNeighbours(uint16_t x, uint16_t y);
};



Life :: Life()
{
  //printValue("Init Life");
  randomSeed(analogRead(0));
  generation = 0;
}



void Life :: createWorld()
{
  //printValue("Creating World");

  // clear display
  backgroundLayer.fillScreen(BLACK);
  delay(2000);

  for (uint16_t x = 0; x < kMatrixWidth; x++)
  {
    for (uint16_t y = 0; y < kMatrixHeight; y++)
    {
      // 15% alive to dead starting ratio
      if (random(100) < 15)
      {
        cells[x][y].alive = true;
        cells[x][y].color = startingColor;
      }
      else
      {
        cells[x][y].alive = false;
        cells[x][y].color = deadColor;
      }

      cells[x][y].prev = cells[x][y].alive;
    }
  }

  generation = 1;
  endCounter = 0;
}



void Life :: updateWorld()
{
  if (generation == 0)
    createWorld();

  // Display current generation
  for (uint16_t x = 0; x < kMatrixWidth; x++)
  {
    for (uint16_t y = 0; y < kMatrixHeight; y++)
    {
      if (!cells[x][y].alive)
        backgroundLayer.drawPixel(x, y, BLACK);
      else
        backgroundLayer.drawPixel(x, y, wheel8(cells[x][y].color));  // was wheel8Sat
    }
  }

  // update display
  backgroundLayer.swapBuffers();
  newCells = 0;

  // Birth and death cycle
  for (uint16_t x = 0; x < kMatrixWidth; x++)
  {
    for (uint16_t y = 0; y < kMatrixHeight; y++)
    {
      uint16_t count = countNeighbours(x, y);

      // new birth ?
      if (count == 3 && cells[x][y].prev == false)
      {
        // A new cell is born
        cells[x][y].alive = true;
        cells[x][y].color = startingColor;
        newCells++;
      }

      // too few or too many neighbors? Die sucker...
      else if ((count < 2 || count > 3) && cells[x][y].prev == true)
      {
        cells[x][y].alive = false;
        cells[x][y].color = deadColor;
      }

      // no change to cell
      else
      {
        if (cells[x][y].color < 253)
          cells[x][y].color += 1;
      }
    }
  }

  //printValue("new cells", newCells);

  // save current generation
  for (uint16_t x = 0; x < kMatrixWidth; x++)
    for (uint16_t y = 0; y < kMatrixHeight; y++)
      cells[x][y].prev = cells[x][y].alive;


  // increment generation counter
  generation++;

  // in work: figure out when to respawn world
  if (generation > maxGenerations)
    generation = 0;

  if (generation % 2 == 0)
  {
    if (newCells == lastCount)
      endCounter++;
    else
      endCounter = 0;

    lastCount = newCells;
  }

  if (endCounter > 8)
  {
    //printValue("generation", generation);
    generation = 0;
  }

  // finally, slow things down a bit
  delay(delayVal * 5);
}


uint16_t Life :: countNeighbours(uint16_t x, uint16_t y)
{
  // origin is top left
  const uint16_t up    = x - 1;
  const uint16_t down  = x + 1;
  const uint16_t right = y + 1;
  const uint16_t left  = y - 1;


  uint16_t n = (cells[(down) % kMatrixWidth][y].prev) +                                                     // down
               (cells[x][(right) % kMatrixHeight].prev) +                                                   // right
               (cells[(up + kMatrixWidth) % kMatrixWidth][y].prev) +                                        // up
               (cells[x][(left + kMatrixHeight) % kMatrixHeight].prev) +                                    // left
               (cells[(down) % kMatrixWidth][(right) % kMatrixHeight].prev) +                               // down rigth
               (cells[(up + kMatrixWidth) % kMatrixWidth][(right) % kMatrixHeight].prev) +                  // up right
               (cells[(up + kMatrixWidth) % kMatrixWidth][(left + kMatrixHeight) % kMatrixHeight].prev) +   // up left
               (cells[(down) % kMatrixWidth][(left + kMatrixHeight) % kMatrixHeight].prev);                 // down left

  //printValue("neighbors", n);
  return n;
}
