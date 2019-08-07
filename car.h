#ifndef CAR_H
#define CAR_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <vector>

#include "point.h"

float const PI = acos(-1), eps = 1e-4;

using namespace std;

class Car {
 private:
  float x;
  float y;
  float radian;
  float tireAngle;
  float v;
  float cameraDelta;
  float cameraZ;
  float staticCount;
  float cameraDistance;

  float const WIDTH = 60, LENGTH = 140, HEIGHT = 40;
  float const MAX_SPEED = 15, TURN_SPEED = 0.15, ACC = 0.075;

 public:
  Car();
  void reset();
  void move(bool *key);
  bool isMoveValid();
  void checkIsTurning(bool *key);
  void moveCamera(float h);
  vector<Point> getPosition();
  void draw();
};
#endif  // !CAR_H