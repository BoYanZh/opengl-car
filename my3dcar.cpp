#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <cmath>
#include <iostream>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

float const PI = acos(-1), eps = 1e-4;

struct car {
  double x = 200;
  double y = 200;
  double radian = PI / 2;
  double tireAngle = 0;
  double vx = 0;
  double vy = 0;
  double delta = 0;
};
struct cpoint {
  double x;
  double y;
};

int const FPS = 60;
float const WIDTH = 60, LENGTH = 140, HEIGHT = 40;
double const MOVE_SPEED = 20, TURN_SPEED = 0.4, ACC = 0.1;
cpoint const outer[26] = {
    {0, 0},          {0, 2310},       {2100, 2310},    {2100, 1680},
    {2310, 1680},    {2310, 1155},    {2100, 1155},    {2100, 840},
    {2100, 840},     {2091.6, 758.1}, {2068.5, 680.4}, {2028.6, 606.9},
    {1976.1, 543.9}, {1913.1, 491.4}, {1841.7, 451.5}, {1761.9, 428.4},
    {1680, 420},     {1598.1, 428.4}, {1520.4, 451.5}, {1446.9, 491.4},
    {1383.9, 541.8}, {1331.4, 606.9}, {1291.5, 678.3}, {1268.4, 758.1},
    {1260, 840},     {1260, 0}};
cpoint const inner[26] = {
    {420, 630},       {420, 1890},      {1050, 1890},     {1050, 1470},
    {1260, 1470},     {1260, 1890},     {1680, 1890},     {1680, 840},
    {1680, 840},      {1671.6, 921.9},  {1648.5, 999.6},  {1608.6, 1073.1},
    {1556.1, 1136.1}, {1493.1, 1188.6}, {1421.7, 1228.5}, {1341.9, 1251.6},
    {1260, 1260},     {1178.1, 1251.6}, {1100.4, 1228.5}, {1026.9, 1188.6},
    {963.9, 1138.2},  {911.4, 1073.1},  {871.5, 1001.7},  {848.4, 921.9},
    {840, 840},       {840, 630}};

bool keyStatus[256];
bool specialStatus[256];

namespace global {
car Car;
float cameraY = HEIGHT * 6;
};  // namespace global
using namespace global;

// check whether two lines are intersectant
bool isLineIntersect(const cpoint &p1, const cpoint &p2, const cpoint &p3,
                     const cpoint &p4) {
  if ((p1.x > p2.x ? p1.x : p2.x) < (p3.x < p4.x ? p3.x : p4.x) ||
      (p1.y > p2.y ? p1.y : p2.y) < (p3.y < p4.y ? p3.y : p4.y) ||
      (p3.x > p4.x ? p3.x : p4.x) < (p1.x < p2.x ? p1.x : p2.x) ||
      (p3.y > p4.y ? p3.y : p4.y) < (p1.y < p2.y ? p1.y : p2.y))
    return false;
  if ((((p1.x - p3.x) * (p4.y - p3.y) - (p1.y - p3.y) * (p4.x - p3.x)) *
       ((p2.x - p3.x) * (p4.y - p3.y) - (p2.y - p3.y) * (p4.x - p3.x))) > 0 ||
      (((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) *
       ((p4.x - p1.x) * (p2.y - p1.y) - (p4.y - p1.y) * (p2.x - p1.x))) > 0)
    return false;
  return true;
}

// check whether two ploygons are intersectant
bool isPloygonIntersect(const cpoint p1[], int n1, const cpoint p2[], int n2) {
  for (int i = 0; i < n1; ++i)
    for (int j = 0; j < n2; ++j)
      if (isLineIntersect(p1[i], p1[(i + 1) % n1], p2[j], p2[(j + 1) % n2]))
        return true;
  return false;
}

// draw a line
void drawLine(const cpoint &p1, const cpoint &p2) {
  double alpha =
      abs(p1.y - p2.y) < eps ? PI / 2 : atan((p1.x - p2.x) / (p1.y - p2.y));
  double const w = 10;
  glBegin(GL_QUADS);
  glVertex3f((p1.x - w * cos(alpha)), p1.y + w * sin(alpha), 1.0f);
  glVertex3f((p2.x - w * cos(alpha)), p2.y + w * sin(alpha), 1.0f);
  glVertex3f((p2.x + w * cos(alpha)), p2.y - w * sin(alpha), 1.0f);
  glVertex3f((p1.x + w * cos(alpha)), p1.y - w * sin(alpha), 1.0f);
  glEnd();
}

// draw a vertical stick
void drawStick(const cpoint &p) {
  for (int j = 0; j < 40; ++j) {
    glBegin(GL_POLYGON);
    glVertex3f(p.x + 10 * cos(2 * PI / 20 * j), p.y + 10 * sin(2 * PI / 20 * j),
               200.0f);
    glVertex3f(p.x + 10 * cos(2 * PI / 20 * j), p.y + 10 * sin(2 * PI / 20 * j),
               0.0f);
    glVertex3f(p.x + 10 * cos(2 * PI / 20 * (j + 1)),
               p.y + 10 * sin(2 * PI / 20 * (j + 1)), 0.0f);
    glVertex3f(p.x + 10 * cos(2 * PI / 20 * (j + 1)),
               p.y + 10 * sin(2 * PI / 20 * (j + 1)), 200.0f);
    glEnd();
  }
}

// draw a polygon
void drawPolygon(const cpoint poly[], const int n,
                 const bool withStick = true) {
  if (!n) return;
  for (int i = 0; i < n - 1; ++i) {
    drawLine(poly[i], poly[i + 1]);
    if (withStick) drawStick(poly[i]);
    glBegin(GL_POLYGON);
    for (int j = 0; j < 20; ++j) {
      glVertex3f(poly[i].x + 10 * cos(2 * PI / 20 * j),
                 poly[i].y + 10 * sin(2 * PI / 20 * j), 1.0f);
    }
    glEnd();
  }
  drawLine(poly[n - 1], poly[0]);
  if (withStick) drawStick(poly[n - 1]);
  glBegin(GL_POLYGON);
  for (int j = 0; j < 20; ++j) {
    glVertex3f(poly[n - 1].x + 10 * cos(2 * PI / 20 * j),
               poly[n - 1].y + 10 * sin(2 * PI / 20 * j), 1.0f);
  }
  glEnd();
}

// get the position of 4 corners of the car
cpoint *getCarPosition() {
  static cpoint re[4];
  float r, r1, delta;
  r = sqrt(WIDTH * WIDTH + LENGTH * LENGTH);
  r1 = LENGTH / 5 * 3;
  delta = atan(WIDTH / LENGTH);
  float angle[] = {delta, PI - delta, PI + delta, 2 * PI - delta};
  for (int i = 0; i < 4; ++i)
    re[i] = {Car.x + r1 * cos(Car.radian) + r * cos(Car.radian + angle[i]),
             Car.y + r1 * sin(Car.radian) + r * sin(Car.radian + angle[i])};
  return re;
}

// draw the green ground of parking lots
void drawParkingLots() {
  // Draw 1st Parking Lot
  glBegin(GL_QUADS);
  glColor3f(0.3f, 1.0f, 0.3f);
  glVertex3f(1050.0f, 1890.0f, 0.5f);
  glVertex3f(1050.0f, 1470.0f, 0.5f);
  glVertex3f(1260.0f, 1470.0f, 0.5f);
  glVertex3f(1260.0f, 1890.0f, 0.5f);
  glEnd();
  // Draw 2nd Parking Lot
  glBegin(GL_QUADS);
  glColor3f(0.3f, 1.0f, 0.3f);
  glVertex3f(2100.0f, 1680.0f, 0.5f);
  glVertex3f(2310.0f, 1680.0f, 0.5f);
  glVertex3f(2310.0f, 1155.0f, 0.5f);
  glVertex3f(2100.0f, 1155.0f, 0.5f);
  glEnd();
}

// draw the start line
void drawStartLine() {
  glColor4f(0.3f, 1.0f, 0.3f, 0.5f);
  glBegin(GL_QUADS);
  glVertex3f(0, 630, 0);
  glVertex3f(0, 630, 200);
  glVertex3f(420, 630, 200);
  glVertex3f(420, 630, 0);
  glEnd();
}

// draw border line of the track
void drawSigns() {
  cpoint *p = getCarPosition();
  glColor3f(0.5f, 0.5f, 0.5f);
  drawPolygon(p, 4, false);
  glColor3f(1.0f, 1.0f, 1.0f);
  if (isPloygonIntersect(p, 4, outer, 26)) glColor3f(1.0f, 0.0f, 0.0f);
  drawPolygon(outer, 26);
  glColor3f(1.0f, 1.0f, 1.0f);
  if (isPloygonIntersect(p, 4, inner, 26)) glColor3f(1.0f, 0.0f, 0.0f);
  drawPolygon(inner, 26);
}

// draw the ground
void drawGround() {
  glBegin(GL_QUADS);
  glColor3f(0.3f, 0.3f, 0.3f);
  glVertex3f(-10000.0f, -10000.0f, -1.0f);
  glVertex3f(-10000.0f, 10000.0f, -1.0f);
  glVertex3f(10000.0f, 10000.0f, -1.0f);
  glVertex3f(10000.0f, -10000.0f, -1.0f);
  glEnd();
}

// draw a car
void drawCar() {
  glPushMatrix();
  double alpha =
      abs(Car.x) < eps ? (-2 * (Car.y >= 0) + 3) * PI / 2 : atan(Car.y / Car.x);
  double r1 = sqrt(Car.x * Car.x + Car.y * Car.y);
  glRotatef(Car.radian / PI * 180, 0.0f, 0.0f, 1.0f);
  glTranslatef(LENGTH / 5 * 3, 0.0f, 0.0f);
  double tmpTheta = Car.x >= 0 ? 0 : PI;
  glTranslatef(r1 * cos(tmpTheta - Car.radian + alpha),
               r1 * sin(tmpTheta - Car.radian + alpha), 0.0f);
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

// check whether the car is turning
void checkIsTurning() {
  int dire = 0;
  if (specialStatus[GLUT_KEY_LEFT] && !specialStatus[GLUT_KEY_RIGHT])
    dire = 1;
  else if (specialStatus[GLUT_KEY_RIGHT] && !specialStatus[GLUT_KEY_LEFT])
    dire = -1;
  Car.tireAngle = dire * 30;
  if (dire) {
    if (Car.vx > 0) {
      Car.delta += dire * 0.03f;
      Car.delta = fmin(Car.delta, PI / 108 * Car.vx);
      Car.delta = fmax(Car.delta, -PI / 108 * Car.vx);
    }
    Car.radian += dire * TURN_SPEED / 2 / PI * Car.vx / MOVE_SPEED;
    Car.radian = fmod(Car.radian + 2 * PI, 2 * PI);
  }
}

// move the car to the position of next instant
void moveCar() {
  int dire = 0;
  if (specialStatus[GLUT_KEY_UP] && !specialStatus[GLUT_KEY_DOWN])
    dire = 1;
  else if (specialStatus[GLUT_KEY_DOWN] && !specialStatus[GLUT_KEY_UP])
    dire = -1;
  if (dire) {
    if (dire * Car.vx >= 0) {  // speed up
      Car.vx += dire * (-2 / 3 * abs(Car.vx) + ACC) * 2;
    } else if (dire * Car.vx < 0) {  // break down
      if (Car.vx > 0)
        Car.vx = max(0.0, Car.vx - (-2 / 3 * abs(Car.vx) + ACC) * 4);
      else if (Car.vx < 0)
        Car.vx = min(0.0, Car.vx + (-2 / 3 * abs(Car.vx) + ACC) * 4);
      // Car.vx += dire * (-2 / 3 * abs(Car.vx) + ACC) * 4;
    }
    Car.vx = max(min(MOVE_SPEED, Car.vx), -MOVE_SPEED / 2);
  } else {
    if (Car.vx > 0) Car.vx = max(0.0, Car.vx - ACC);
    if (Car.vx < 0) Car.vx = min(0.0, Car.vx + ACC);
  }
  if (Car.delta > 0) Car.delta = fmax(0.0, Car.delta - 0.015f);
  if (Car.delta < 0) Car.delta = fmin(0.0, Car.delta + 0.015f);
  if (Car.vx) checkIsTurning();
  Car.x += cos(Car.radian) * Car.vx;
  Car.y += sin(Car.radian) * Car.vx;
}

// START: recall functions for GLUT

void drawScene() {
  drawStartLine();
  drawParkingLots();
  drawSigns();
  drawGround();
}

void keyPressed(unsigned char key, int xx, int yy) {
  if (key == 27) exit(0);
  keyStatus[key] = true;
}

void keyUp(unsigned char key, int x, int y) { keyStatus[key] = false; }

void specialPressed(int key, int x, int y) { specialStatus[key] = true; }

void specialUp(int key, int x, int y) { specialStatus[key] = false; }

void renderScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  float distance = WIDTH * 8;  // + (Car.vx > 0 ? Car.vx * WIDTH / 4 : 0);
  gluLookAt(Car.x - distance * cos(Car.radian - Car.delta),
            Car.y - distance * sin(Car.radian - Car.delta), cameraY, Car.x,
            Car.y, 10, 0.0f, 0.0f, 1.0f);
  drawCar();
  drawScene();
  glutSwapBuffers();
}

void Initialization() {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GLUT_MULTISAMPLE);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glEnable(GL_SMOOTH);
}

void changeSize(int w, int h) {
  if (h == 0) h = 1;
  float ratio = w * 1.0 / h;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(45.0f, ratio, 0.1f, 10000.0f);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.3398, 0.9803, 1.0, 0);
  Initialization();
}

void timerFunc(int value) {
  moveCar();
  if (keyStatus[(int)'w']) cameraY += 5.0f;
  if (keyStatus[(int)'s']) cameraY -= 5.0f;
  glutPostRedisplay();
  glutTimerFunc(1000 / FPS, timerFunc, 1);
}

// END: recall functions for GLUT

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(800, 600);
  glutCreateWindow("My 3D Car");

  Initialization();

  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  // glutIdleFunc(renderScene);

  glutKeyboardFunc(keyPressed);
  glutKeyboardUpFunc(keyUp);
  glutSpecialFunc(specialPressed);
  glutSpecialUpFunc(specialUp);
  glutTimerFunc(1000 / FPS, timerFunc, 1);

  glutMainLoop();

  return 1;
}