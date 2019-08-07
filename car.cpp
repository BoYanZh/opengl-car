#include <iostream>
#include <vector>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "car.h"
#include "util.hpp"

Car::Car() { reset(); }

void Car::reset() {
  x = 200;
  y = 200;
  radian = PI / 2;
  tireAngle = 0;
  v = 0;
  cameraDelta = 0;
  staticCount = 20;
  cameraZ = HEIGHT * 6;
}

// move the car to the position of next instant
void Car::move(bool *key) {
  int dire = 0;
  if (key[GLUT_KEY_UP] && !key[GLUT_KEY_DOWN])
    dire = 1;
  else if (key[GLUT_KEY_DOWN] && !key[GLUT_KEY_UP])
    dire = -1;
  if (staticCount > 0) staticCount--, dire = 0;
  if (dire) {
    if (dire * v >= 0) {  // speed up
      v += dire * (-2 / 3 * abs(v) + ACC) * 2;
    } else if (dire * v < 0) {  // break down
      if (v > 0)
        v = fmax(0.0, v - (-2 / 3 * abs(v) + ACC) * 6);
      else if (v < 0)
        v = fmin(0.0, v + (-2 / 3 * abs(v) + ACC) * 6);
      if (abs(v) < eps) staticCount = 15;
    }
    v = fmax(fmin(MAX_SPEED, v), -MAX_SPEED / 2);
  } else {
    if (v > 0) v = fmax(0.0, v - ACC * 2);
    if (v < 0) v = fmin(0.0, v + ACC * 2);
  }
  if (cameraDelta > 0) cameraDelta = fmax(0.0, cameraDelta - 0.01f);
  if (cameraDelta < 0) cameraDelta = fmin(0.0, cameraDelta + 0.01f);
  if (v) checkIsTurning(key);
  cameraDistance = WIDTH * 8;  // + (v > 0 ? v * WIDTH / 4 : 0);
  x += cos(radian) * v, y += sin(radian) * v;
}

// check whether the car is turning
void Car::checkIsTurning(bool *key) {
  int dire = 0;
  if (key[GLUT_KEY_LEFT] && !key[GLUT_KEY_RIGHT])
    dire = 1;
  else if (key[GLUT_KEY_RIGHT] && !key[GLUT_KEY_LEFT])
    dire = -1;
  tireAngle = dire * 30;
  if (dire) {
    if (v > 0) {
      cameraDelta += dire * 0.011f;
      cameraDelta = fmin(cameraDelta, PI / 270 * v);
      cameraDelta = fmax(cameraDelta, -PI / 270 * v);
    }
  }
  radian += dire * TURN_SPEED / PI * v / MAX_SPEED;
  radian = fmod(radian + 2 * PI, 2 * PI);
}

// draw a car
void Car::draw() {
  gluLookAt(x - cameraDistance * cos(radian - cameraDelta),
            y - cameraDistance * sin(radian - cameraDelta), cameraZ, x, y,
            HEIGHT / 2, 0.0f, 0.0f, 1.0f);
  glPushMatrix();
  float alpha = abs(x) < eps ? (-2 * (y >= 0) + 3) * PI / 2 : atan(y / x);
  float r1 = sqrt(x * x + y * y);
  glRotatef(radian / PI * 180, 0.0f, 0.0f, 1.0f);
  glTranslatef(LENGTH / 5 * 3, 0.0f, 0.0f);
  float tmpTheta = x >= 0 ? 0 : PI;
  glTranslatef(r1 * cos(tmpTheta - radian + alpha),
               r1 * sin(tmpTheta - radian + alpha), 0.0f);
  {
    glBegin(GL_POLYGON);
    glColor3f(0.8, 0.5, 0.2);
    glVertex3f(LENGTH, -WIDTH, HEIGHT);
    glVertex3f(-LENGTH, -WIDTH, HEIGHT);
    glVertex3f(-LENGTH, WIDTH, HEIGHT);
    glVertex3f(LENGTH, WIDTH, HEIGHT);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0, 0, 1);
    glVertex3f(LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, 2 * HEIGHT);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(-LENGTH, -WIDTH, HEIGHT);
    glVertex3f(-LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, WIDTH, HEIGHT);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(LENGTH, -WIDTH, HEIGHT);
    glVertex3f(LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, HEIGHT);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(-LENGTH, WIDTH, HEIGHT);
    glVertex3f(-LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, HEIGHT);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(-LENGTH, -WIDTH, HEIGHT);
    glVertex3f(-LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, -WIDTH, HEIGHT);
    glEnd();
  }
  glPopMatrix();
}

// get the position of 4 corners of the car
vector<Point> Car::getPosition() {
  static vector<Point> re(4);
  float r, r1, delta;
  r = sqrt(WIDTH * WIDTH + LENGTH * LENGTH);
  r1 = LENGTH / 5 * 3;
  delta = atan(WIDTH / LENGTH);
  float angle[] = {delta, PI - delta, PI + delta, 2 * PI - delta};
  for (int i = 0; i < 4; ++i)
    re[i] = {x + r1 * cos(radian) + r * cos(radian + angle[i]),
             y + r1 * sin(radian) + r * sin(radian + angle[i])};
  return re;
}

void Car::moveCamera(float h) {
  cameraDistance = fmax(0.01, cameraDistance - h);
  cameraZ = fmax(0, cameraZ + h);
}