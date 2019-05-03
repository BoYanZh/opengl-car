#include <GL/glut.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
using namespace std;

bool specialStatus[256];
int const FPS = 60;
float const PI = 3.14;
double const MOVE_SPEED = 20, TURN_SPEED = 0.4, ACC = 0.1;
const float WIDTH = 60, LENGTH = 180, HEIGHT = 40;
struct car {
  double x = -300;
  double y = 300;
  double radian = PI;
  double tireAngle = 0;
  double vx = 0;
  double vy = 0;
  double delta = 0;
  int staticCount = 0;
};
struct cpoint {
  double x;
  double y;
};

car myCar;

float y = HEIGHT * 3;

cpoint poly[9] = {{0, 0},       {0, 1000},    {500, 1500},
                  {1500, 1500}, {2000, 1000}, {2000, 0},
                  {1500, -500}, {500, -500},  {0, 0}};
cpoint outer[27] = {{0, 0},       {0, 11},      {10, 11},     {10, 8},
                    {11, 8},      {11, 5.5},      {10, 5.5},      {10, 4},
                    {10.0, 4.0},  {9.96, 3.61}, {9.85, 3.24}, {9.66, 2.89},
                    {9.41, 2.59}, {9.11, 2.34}, {8.77, 2.15}, {8.39, 2.04},
                    {8.0, 2.0},   {7.61, 2.04}, {7.24, 2.15}, {6.89, 2.34},
                    {6.59, 2.58}, {6.34, 2.89}, {6.15, 3.23}, {6.04, 3.61},
                    {6, 4},       {6, 0},       {0, 0}};
cpoint inner[27] = {{2, 3},       {2, 9},       {5, 9},       {5, 7},
                    {6, 7},       {6, 9},       {8, 9},       {8, 4},
                    {8.0, 4.0},   {7.96, 4.39}, {7.85, 4.76}, {7.66, 5.11},
                    {7.41, 5.41}, {7.11, 5.66}, {6.77, 5.85}, {6.39, 5.96},
                    {6.0, 6.0},   {5.61, 5.96}, {5.24, 5.85}, {4.89, 5.66},
                    {4.59, 5.42}, {4.34, 5.11}, {4.15, 4.77}, {4.04, 4.39},
                    {4, 4},       {4, 3},       {2, 3}};

bool complInsideConvex(const cpoint &p, cpoint *con, int n) {
  int j = n - 1, res = 0;
  for (int i = 0; i < n; i++) {
    if (((con[i].y < p.y && con[j].y >= p.y) ||
         (con[j].y < p.y && con[i].y >= p.y)) &&
        (con[i].x <= p.x || con[j].x <= p.x)) {
      res ^= ((con[i].x + (p.y - con[i].y) / (con[j].y - con[i].y) *
                              (con[j].x - con[i].x)) < p.x);
    }
    j = i;
  }
  return res;
}

void changeSize(int w, int h) {
  // Prevent a divide by zero, when window is too short
  // (you cant make a window of zero width).
  if (h == 0) h = 1;
  float ratio = w * 1.0 / h;
  // Use the Projection Matrix
  glMatrixMode(GL_PROJECTION);
  // Reset Matrix
  glLoadIdentity();
  // Set the viewport to be the entire window
  glViewport(0, 0, w, h);
  // Set the correct perspective.
  gluPerspective(45.0f, ratio, 0.1f, 10000.0f);
  // Get Back to the Modelview
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.3398, 0.9803, 1.0, 0);
}

void drawLine(const cpoint p1, const cpoint p2) {
  double alpha =
      p1.y - p2.y == 0 ? PI / 2 : atan((p1.x - p2.x) / (p1.y - p2.y));
  double const w = 10;
  glBegin(GL_QUADS);
  glVertex3f(-(p1.x - w * cos(alpha)), 0.5f, p1.y + w * sin(alpha));
  glVertex3f(-(p2.x - w * cos(alpha)), 0.5f, p2.y + w * sin(alpha));
  glVertex3f(-(p2.x + w * cos(alpha)), 0.5f, p2.y - w * sin(alpha));
  glVertex3f(-(p1.x + w * cos(alpha)), 0.5f, p1.y - w * sin(alpha));
  glEnd();
}

void drawPolygon(const cpoint poly[], const int n) {
  if (!n) return;
  for (int i = 0; i < n; ++i) {
    drawLine(poly[i], poly[i + 1]);
    glBegin(GL_POLYGON);
    for (int j = 0; j < 20; ++j) {
      glVertex3f(-(poly[i].x + 10 * cos(2 * PI / n * j)), 0.5f,
                 poly[i].y + 10 * sin(2 * PI / n * j));
    }
    glEnd();
  }
}

void drawCar() {
  // cout << myCar.radian * 180 / PI << ' ' << myCar.y << ' ' << myCar.x <<
  // endl;
  glPushMatrix();
  glRotatef(myCar.radian * 180 / PI, 0.0f, 1.0f, 0.0f);
  double alpha = myCar.y == 0 ? PI / 2 : atan(myCar.x / myCar.y);
  double r1 = sqrt(myCar.x * myCar.x + myCar.y * myCar.y);
  if (myCar.y < 0)
    glTranslatef(r1 * cos(alpha + myCar.radian), HEIGHT * 1.5 + 10,
                 r1 * sin(alpha + myCar.radian));
  else
    glTranslatef(-r1 * cos(alpha + myCar.radian), HEIGHT * 1.5 + 10,
                 -r1 * sin(alpha + myCar.radian));
  // dire
  glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
  //车下半部 左
  glBegin(GL_POLYGON);
  glColor3f(0.1, 0.1, 0.8);
  glVertex3f(-LENGTH, 0.0f, WIDTH);
  glVertex3f(LENGTH, 0.0f, WIDTH);
  // glColor3f(1, 0.5, 0.8);
  glVertex3f(LENGTH, -HEIGHT, WIDTH);
  glVertex3f(-LENGTH, -HEIGHT, WIDTH);
  glEnd();
  //车下半部 后
  glBegin(GL_POLYGON);
  glColor3f(0.3, 0.2, 0.5);
  glVertex3f(LENGTH, 0.0f, -WIDTH);
  glVertex3f(LENGTH, 0.0f, WIDTH);
  glVertex3f(LENGTH, -HEIGHT, WIDTH);
  glVertex3f(LENGTH, -HEIGHT, -WIDTH);
  glEnd();
  //车下半部 前
  glBegin(GL_POLYGON);
  glColor3f(0.3, 0.1, 0.3);
  glVertex3f(-LENGTH, 0.0f, -WIDTH);
  glVertex3f(-LENGTH, 0.0f, WIDTH);
  glVertex3f(-LENGTH, -HEIGHT, WIDTH);
  glVertex3f(-LENGTH, -HEIGHT, -WIDTH);
  glEnd();
  //车下半部 右
  glBegin(GL_POLYGON);
  glColor3f(0.1, 0.1, 0.8);
  glVertex3f(-LENGTH, 0.0f, -WIDTH);
  glVertex3f(LENGTH, 0.0f, -WIDTH);
  // glColor3f(1, 0.5, 0.8);
  glVertex3f(LENGTH, -HEIGHT, -WIDTH);
  glVertex3f(-LENGTH, -HEIGHT, -WIDTH);
  glEnd();
  //车下半部 上
  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(-LENGTH, 0.0f, WIDTH);
  glVertex3f(-LENGTH, 0.0f, -WIDTH);
  glVertex3f(LENGTH, 0.0f, -WIDTH);
  glVertex3f(LENGTH, 0.0f, WIDTH);
  glEnd();
  //车下半部 下
  glBegin(GL_POLYGON);
  glColor3f(0.8, 0.5, 0.2);
  glVertex3f(-LENGTH, -HEIGHT, WIDTH);
  glVertex3f(-LENGTH, -HEIGHT, -WIDTH);
  glVertex3f(LENGTH, -HEIGHT, -WIDTH);
  glVertex3f(LENGTH, -HEIGHT, WIDTH);
  glEnd();
  //车上半部 左
  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(-LENGTH / 2, 0.0f, WIDTH);
  glVertex3f(-LENGTH / 4, HEIGHT, WIDTH);
  glVertex3f(LENGTH / 2, HEIGHT, WIDTH);
  glVertex3f(LENGTH / 8 * 5, 0.0f, WIDTH);
  glEnd();
  //车上半部 右
  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(-LENGTH / 2, 0.0f, -WIDTH);
  glVertex3f(-LENGTH / 4, HEIGHT, -WIDTH);
  glVertex3f(LENGTH / 2, HEIGHT, -WIDTH);
  glVertex3f(LENGTH / 8 * 5, 0.0f, -WIDTH);
  glEnd();
  //车上半部 上
  glBegin(GL_POLYGON);
  glColor3f(0.3, 0.2, 0.5);
  glVertex3f(-LENGTH / 4, HEIGHT, WIDTH);
  glVertex3f(-LENGTH / 4, HEIGHT, -WIDTH);
  glVertex3f(LENGTH / 2, HEIGHT, -WIDTH);
  glVertex3f(LENGTH / 2, HEIGHT, WIDTH);
  glEnd();
  //车上半部 前
  glBegin(GL_POLYGON);
  glColor3f(0.5, 0.8, 0.8);
  glVertex3f(-LENGTH / 4, HEIGHT, WIDTH);
  glVertex3f(-LENGTH / 2, 0.0f, WIDTH);
  glVertex3f(-LENGTH / 2, 0.0f, -WIDTH);
  glVertex3f(-LENGTH / 4, HEIGHT, -WIDTH);
  glEnd();
  // //车上半部 后
  glBegin(GL_POLYGON);
  glColor3f(0, 0.5, 0.5);
  glVertex3f(LENGTH / 2, HEIGHT, WIDTH);
  glVertex3f(LENGTH / 2, HEIGHT, -WIDTH);
  glVertex3f(LENGTH / 8 * 5, 0.0f, -WIDTH);
  glVertex3f(LENGTH / 8 * 5, 0.0f, WIDTH);
  glEnd();
  //车轮
  float pos[4][3] = {{-LENGTH / 2, -HEIGHT, WIDTH},
                     {-LENGTH / 2, -HEIGHT, -WIDTH},
                     {LENGTH / 8 * 5, -HEIGHT, WIDTH},
                     {LENGTH / 8 * 5, -HEIGHT, -WIDTH}};
  for (int i = 0; i < 4; ++i) {
    glPushMatrix();
    glTranslated(pos[i][0], pos[i][1], pos[i][2]);
    if (i < 2) glRotatef(myCar.tireAngle, 0.0f, 1.0f, 0.0f);
    glColor3f(0, 0, 0);
    glutSolidTorus(10, HEIGHT / 2, 5, 100);
    glColor3f(0.2, 0.2, 0.2);
    glutSolidTorus(10, 10, 5, 100);
    glPopMatrix();
  }
  glPopMatrix();
}

void drawGround() {
  // Draw ground
  glBegin(GL_QUADS);
  glColor3f(0.3f, 0.3f, 0.3f);
  glVertex3f(-10000.0f, 0.0f, -10000.0f);
  glVertex3f(-10000.0f, 0.0f, 10000.0f);
  glVertex3f(10000.0f, 0.0f, 10000.0f);
  glVertex3f(10000.0f, 0.0f, -10000.0f);
  glEnd();
  // FIX: COLLISION BOX NOT CORRECT
  cpoint p[4] = {{myCar.y + WIDTH, -myCar.x - LENGTH},
                 {myCar.y + WIDTH, -myCar.x + LENGTH},
                 {myCar.y - WIDTH, -myCar.x + LENGTH},
                 {myCar.y - WIDTH, -myCar.x - LENGTH}};
  glColor3f(1.0f, 1.0f, 1.0f);
  for (int i = 0; i < 4; ++i) {
    if (!complInsideConvex(p[i], outer, 26)) {
      glColor3f(1.0f, 0.0f, 0.0f);
      break;
    }
  }
  drawPolygon(outer, 26);
  glColor3f(1.0f, 1.0f, 1.0f);
  for (int i = 0; i < 4; ++i) {
    if (complInsideConvex(p[i], inner, 26)) {
      glColor3f(1.0f, 0.0f, 0.0f);
      break;
    }
  }
  drawPolygon(inner, 26);
}

void renderScene(void) {
  // Clear Color and Depth Buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Reset transformations
  glLoadIdentity();
  // Set the camera
  float distance = WIDTH * 8 + (myCar.vx > 0 ? myCar.vx * WIDTH / 4 : 0);
  gluLookAt(-myCar.y + distance * sin(myCar.radian - myCar.delta), y,
            -myCar.x + distance * cos(myCar.radian - myCar.delta), -myCar.y,
            HEIGHT, -myCar.x, 0.0f, 1.0f, 0.0f);

  drawCar();
  drawGround();
  glutSwapBuffers();
}

void pressKey(unsigned char key, int xx, int yy) {
  switch (key) {
    case 'w':
      y += 5.0f;
      break;
    case 's':
      y -= 5.0f;
      break;
  }
}

void releaseKey(unsigned char key, int x, int y) {
  switch (key) {
    case 'w':
    case 's':
      break;
  }
}

void specialPressed(int key, int x, int y) { specialStatus[key] = true; }

void specialUp(int key, int x, int y) { specialStatus[key] = false; }

void checkTurn() {
  int dire = 0;
  if (specialStatus[GLUT_KEY_LEFT] && !specialStatus[GLUT_KEY_RIGHT])
    dire = 1;
  else if (specialStatus[GLUT_KEY_RIGHT] && !specialStatus[GLUT_KEY_LEFT])
    dire = -1;
  myCar.tireAngle = dire * 30;
  if (dire) {
    if (myCar.vx > 0) {
      myCar.delta += dire * 0.03f;
      myCar.delta = fmin(myCar.delta, PI / 108 * myCar.vx);
      myCar.delta = fmax(myCar.delta, -PI / 108 * myCar.vx);
    }
    myCar.radian += dire * TURN_SPEED / 2 / PI * myCar.vx / MOVE_SPEED;
    myCar.radian = fmod(myCar.radian + 2 * PI, 2 * PI);
  }
}

void moveCar() {
  int dire = 0;
  if (specialStatus[GLUT_KEY_UP] && !specialStatus[GLUT_KEY_DOWN])
    dire = 1;
  else if (specialStatus[GLUT_KEY_DOWN] && !specialStatus[GLUT_KEY_UP])
    dire = -1;
  if (dire) {
    if (dire * myCar.vx >= 0 && !myCar.staticCount) {  // speed up
      myCar.vx += dire * (-2 / 3 * abs(myCar.vx) + ACC) * 2;
    } else if (dire * myCar.vx < 0) {  // break down
      if (myCar.vx > 0)
        myCar.vx = max(0.0, myCar.vx - (-2 / 3 * abs(myCar.vx) + ACC) * 4);
      if (myCar.vx < 0)
        myCar.vx = min(0.0, myCar.vx + (-2 / 3 * abs(myCar.vx) + ACC) * 4);
      if (abs(myCar.vx) < 1e4) myCar.staticCount = 15;
      // myCar.vx += dire * (-2 / 3 * abs(myCar.vx) + ACC) * 4;
    }
    myCar.vx = max(min(MOVE_SPEED, myCar.vx), -MOVE_SPEED / 2);
  } else {
    if (myCar.vx > 0) myCar.vx = max(0.0, myCar.vx - ACC);
    if (myCar.vx < 0) myCar.vx = min(0.0, myCar.vx + ACC);
  }
  if (myCar.staticCount) --myCar.staticCount;
  if (myCar.delta > 0) myCar.delta = fmax(0.0, myCar.delta - 0.015f);
  if (myCar.delta < 0) myCar.delta = fmin(0.0, myCar.delta + 0.015f);
  if (myCar.vx) checkTurn();
  myCar.x += cos(myCar.radian) * myCar.vx;
  myCar.y += sin(myCar.radian) * myCar.vx;
}

void timerFunc(int value) {
  moveCar();
  glutPostRedisplay();
  glutTimerFunc(1000 / FPS, timerFunc, 1);
}

int main(int argc, char **argv) {
  for (auto &c : outer) {
    c.x *= 210;
    c.y *= 210;
  }
  for (auto &c : inner) {
    c.x *= 210;
    c.y *= 210;
  }
  // init GLUT and create window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(800, 600);
  glutCreateWindow("My 3D Car");

  // register callbacks
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  // glutIdleFunc(renderScene);

  // glutIgnoreKeyRepeat(1);
  // glutKeyboardFunc(processNormalKeys);
  glutKeyboardFunc(pressKey);
  glutKeyboardUpFunc(releaseKey);
  // glutSpecialFunc(pressKey);
  // glutSpecialUpFunc(releaseKey);
  glutSpecialFunc(specialPressed);
  glutSpecialUpFunc(specialUp);

  // OpenGL init
  glEnable(GL_DEPTH_TEST);

  // Timer init
  glutTimerFunc(1000 / FPS, timerFunc, 1);

  // enter GLUT event processing cycle
  glutMainLoop();

  return 1;
}