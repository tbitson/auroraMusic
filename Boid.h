/********************************************************************************

   Aurora: https://github.com/pixelmatix/aurora
   Copyright (c) 2014 Jason Coon

   Modifications to support larger matrix sizes Apr 2021 tbitson

   Portions of this code are adapted from "Flocking" in
   "The Nature of Code" by Daniel Shiffman: http://natureofcode.com/
   Copyright (c) 2014 Daniel Shiffman
   http://www.shiffman.net

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
   the Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
   FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
   COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

********************************************************************************/


#pragma once





class Boid {

  public:
    PVector location;
    PVector velocity;
    PVector acceleration;
    float maxforce;    // Maximum steering force
    float maxspeed;    // Maximum speed
    float desiredseparation = 4;
    float neighbordist = 8;
    float mass;
    float limitX;     // width (top is 0)
    float limitY;     // height (left is 0)



    Boid()
    {    }


    Boid(float x, float y)
    {
      acceleration = PVector(0, 0);
      velocity = PVector(randomf(), randomf());
      location = PVector(x, y);
      maxspeed = 1.5;
      maxforce = 0.05;
    }


    // generate a semi-random float
    static float randomf()
    {
      return mapfloat(random(0, 255), 0, 255, -.5, .5);
    }

    // map a float to a range
    static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
    {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }



    // main entry point
    void run(Boid boids [], uint8_t boidCount)
    {
      flock(boids, boidCount);
      update();
    }



    // Method to update location
    void update()
    {
      // Update velocity
      velocity += acceleration;

      // Limit speed
      velocity.limit(maxspeed);
      location += velocity;

      // Reset acceleration to 0 each cycle
      acceleration *= 0;
    }



    void applyForce(PVector force)
    {
      // We could add mass here if we want A = F / M
      acceleration += force;
    }



    void repelForce(PVector obstacle, float radius)
    {
      //Force that drives boid away from obstacle.

      //Calculate future position for more effective behavior.
      PVector futPos = location + velocity;
      PVector dist = obstacle - futPos;
      float d = dist.mag();

      if (d <= radius)
      {
        PVector repelVec = location - obstacle;
        repelVec.normalize();

        //Don't divide by zero.
        if (d != 0)
        {
          // float scale = 1.0 / d; //The closer to the obstacle, the stronger the force.
          repelVec.normalize();
          repelVec *= (maxforce * 7);
          if (repelVec.mag() < 0)
          {
            //Don't let the boids turn around to avoid the obstacle.
            repelVec.y = 0;
          }
        }
        applyForce(repelVec);
      }
    }



    // We accumulate a new acceleration each time based on three rules
    void flock(Boid boids [], uint8_t boidCount)
    {
      PVector sep = separate(boids, boidCount);   // Separation
      PVector ali = align(boids, boidCount);      // Alignment
      PVector coh = cohesion(boids, boidCount);   // Cohesion

      // Arbitrarily weight these forces
      sep *= 1.5;
      ali *= 1.0;
      coh *= 1.0;

      // Add the force vectors to acceleration
      applyForce(sep);
      applyForce(ali);
      applyForce(coh);
    }



    // Separation
    // Method checks for nearby boids and steers away
    PVector separate(Boid boids [], uint8_t boidCount)
    {
      PVector steer = PVector(0, 0);
      int count = 0;

      // For every boid in the system, check if it's too close
      for (int i = 0; i < boidCount; i++)
      {
        Boid other = boids[i];

        float d = location.dist(other.location);
        // If the distance is greater than 0 and less
        // than an arbitrary amount (0 when you are yourself)
        if ((d > 0) && (d < desiredseparation))
        {
          // Calculate vector pointing away from neighbor
          PVector diff = location - other.location;
          diff.normalize();
          diff /= d;        // Weight by distance
          steer += diff;
          count++;            // Keep track of how many
        }
      }

      // Average -- divide by how many
      if (count > 0)
        steer /= (float) count;

      // As long as the vector is greater than 0
      if (steer.mag() > 0)
      {
        // Implement Reynolds: Steering = Desired - Velocity
        steer.normalize();
        steer *= maxspeed;
        steer -= velocity;
        steer.limit(maxforce);
      }
      return steer;
    }



    // Alignment
    // For every nearby boid in the system, calculate the average velocity
    PVector align(Boid boids [], uint8_t boidCount)
    {
      PVector sum = PVector(0, 0);
      int count = 0;
      for (int i = 0; i < boidCount; i++)
      {
        Boid other = boids[i];

        float d = location.dist(other.location);
        if ((d > 0) && (d < neighbordist))
        {
          sum += other.velocity;
          count++;
        }
      }

      if (count > 0)
      {
        sum /= (float) count;
        sum.normalize();
        sum *= maxspeed;
        PVector steer = sum - velocity;
        steer.limit(maxforce);
        return steer;
      }
      else
        return PVector(0, 0);
    }



    // Cohesion
    // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
    PVector cohesion(Boid boids [], uint8_t boidCount)
    {
      // Start with empty vector to accumulate all locations
      PVector sum = PVector(0, 0);
      int count = 0;
      for (int i = 0; i < boidCount; i++)
      {
        Boid other = boids[i];

        float d = location.dist(other.location);
        if ((d > 0) && (d < neighbordist))
        {
          // Add location
          sum += other.location;
          count++;
        }
      }
      if (count > 0)
      {
        sum /= count;
        return seek(sum);  // Steer towards the location
      }
      else
        return PVector(0, 0);
    }



    // A method that calculates and applies a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    PVector seek(PVector target)
    {
      // A vector pointing from the location to the target
      PVector desired = target - location;
      // Normalize desired and scale to maximum speed
      desired.normalize();
      desired *= maxspeed;
      // Steering = Desired minus Velocity
      PVector steer = desired - velocity;
      steer.limit(maxforce);  // Limit to maximum steering force
      return steer;
    }



    // A method that calculates a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    void arrive(PVector target)
    {
      // A vector pointing from the location to the target
      PVector desired = target - location;

      float d = desired.mag();
      // Normalize desired and scale with arbitrary damping within 100 pixels
      desired.normalize();

      if (d < 4)
      {
        float m = map(d, 0, 100, 0, maxspeed);
        desired *= m;
      }
      else
        desired *= maxspeed;

      // Steering = Desired minus Velocity
      PVector steer = desired - velocity;
      steer.limit(maxforce);  // Limit to maximum steering force
      applyForce(steer);
    }



    void wrapAroundBorders()
    {
      if (location.x < 0)
        location.x = limitX - 1;
      else if (location.x > limitX)
        location.x = 0;

      if (location.y < 0)
        location.y = limitY - 1;
      else if (location.y > limitY)
        location.y = 0;
    }



    void avoidBorders()
    {
      PVector desired = velocity;

      if (location.x < 8)
        desired = PVector(maxspeed, velocity.y);
      else if (location.x > limitX - 8)
        desired = PVector(-maxspeed, velocity.y);

      if (location.y < 8)
        desired = PVector(velocity.x, maxspeed);
      else if (location.y > limitY - 8)
        desired = PVector(velocity.x, -maxspeed);

      if (desired != velocity)
      {
        PVector steer = desired - velocity;
        steer.limit(maxforce);
        applyForce(steer);
      }

      if (location.x < 0)
        location.x = 0;
      else if (location.x > limitX)
        location.x = limitX - 1;

      if (location.y < 0)
        location.y = 0;
      else if (location.y > limitY)
        location.y = limitY - 1;
    }



    void bounceOffBorders(float bounce)
    {
      if (location.x > limitX)
      {
        location.x = limitX - 1;
        velocity.x *= -bounce;
        Serial.println("x bounce limit");
      }
      else if (location.x < 0)
      {
        location.x = 0;
        velocity.x *= -bounce;
        Serial.println("x bounce 0");
      }

      if (location.y > limitY)
      {
        location.y = limitY - 1;
        velocity.y *= -bounce;
      }
      else if (location.y < 0)
      {
        location.y = 0;
        velocity.y *= -bounce;
      }
    }


    void moveThroughBorders(float bounce, float upperLimitY, float lowerLimitY)
    {
      if (location.x > limitX)
      {
        location.x = limitX - 1;
        velocity.x *= -bounce;
        Serial.println("x bounce limit");
      }
      else if (location.x < 0)
      {
        location.x = 0;
        velocity.x *= -bounce;
        Serial.println("x bounce 0");
      }

      // bottom
      if (location.y > upperLimitY)
      {
        if (location.y > kScreenHeight - 1)
          location.y = kScreenHeight - 1;

        velocity.y *= -bounce;
      }
      // top
      else if (location.y < lowerLimitY)
      {
        location.y = 0;
        velocity.y *= -bounce;
      }
    }
};
