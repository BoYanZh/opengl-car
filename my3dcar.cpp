#include <GL/glut.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
using namespace std;

bool specialStatus[256];
int const FPS = 60;
float const PI = 3.14;
double const MOVE_SPEED = 10, TURN_SPEED = 0.3, ACC = 0.1;
struct car {
  const double WIDTH = 15;
  const double LENGTH = 40;
  double x = 0;
  double y = 0;
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

float y = 50.0f;

double cross(cpoint p0, cpoint p1, cpoint p2) {
  return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

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

void drawTrack(const float x[], const float y[], const int n, int debug = 0) {
  // double const w = 50 * 1.414;
  // float tmpx[n + 1], tmpy[n + 1], alpha[n + 1], tmpAlpha;
  // for (int i = 0; i <= n; ++i) {
  //   if (abs(x[i] - x[i + 1]) < 1e-2) {
  //     alpha[i] = y[i + 1] > y[i] ? 0 : PI;
  //   } else
  //     alpha[i] = abs(y[i] - y[i + 1]) < 1e-2
  //                    ? PI / 2 * (-1 + 2 * (x[i] < x[i + 1]))
  //                    : atan((x[i] - x[i + 1]) / (y[i] - y[i + 1]));
  //   if (y[i] - y[i + 1] > 0) alpha[i] += PI;
  //   if (debug) cout << alpha[i] / PI * 180 << ' ';
  // }
  // if (debug) cout << endl;
  // for (int i = 1; i <= n; ++i) {
  //   tmpAlpha = (fmod(PI + alpha[i - 1], 2 * PI) + alpha[i]) / 2;
  //   tmpx[i] = x[i] + w * sin(tmpAlpha);
  //   tmpy[i] = y[i] + w * cos(tmpAlpha);
  //   if (debug)
  //     cout << tmpAlpha / PI * 180 << ' ' << x[i] << ' ' << y[i] << ' '
  //          << tmpx[i] << ' ' << tmpy[i] << endl;
  // }
  // tmpx[0] = tmpx[n];
  // tmpy[0] = tmpy[n];
  // drawPolygon(tmpx, tmpy, n);
}

void drawCar() {
  // cout << myCar.radian * 180 / PI << ' ' << myCar.y << ' ' << myCar.x <<
  // endl;
  glPushMatrix();
  glRotatef(myCar.radian * 180 / PI, 0.0f, 1.0f, 0.0f);
  double alpha = myCar.y == 0 ? PI / 2 : atan(myCar.x / myCar.y);
  double r1 = sqrt(myCar.x * myCar.x + myCar.y * myCar.y);
  if (myCar.y < 0)
    glTranslatef(r1 * cos(alpha + myCar.radian), 22.5f,
                 r1 * sin(alpha + myCar.radian));
  else
    glTranslatef(-r1 * cos(alpha + myCar.radian), 22.5f,
                 -r1 * sin(alpha + myCar.radian));
  // dire
  glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
  //车下半部 左
  glBegin(GL_POLYGON);
  glColor3f(0.1, 0.1, 0.8);
  glVertex3f(-40.0f, 0.0f, 15.0f);
  glVertex3f(40.0f, 0.0f, 15.0f);
  // glColor3f(1, 0.5, 0.8);
  glVertex3f(40.0f, -15.0f, 15.0f);
  glVertex3f(-40.0f, -15.0f, 15.0f);
  glEnd();
  //车下半部 后
  glBegin(GL_POLYGON);
  glColor3f(0.3, 0.2, 0.5);
  glVertex3f(40.0f, 0.0f, -15.0f);
  glVertex3f(40.0f, 0.0f, 15.0f);
  glVertex3f(40.0f, -15.0f, 15.0f);
  glVertex3f(40.0f, -15.0f, -15.0f);
  glEnd();
  //车下半部 前
  glBegin(GL_POLYGON);
  glColor3f(0.3, 0.1, 0.3);
  glVertex3f(-40.0f, 0.0f, -15.0f);
  glVertex3f(-40.0f, 0.0f, 15.0f);
  glVertex3f(-40.0f, -15.0f, 15.0f);
  glVertex3f(-40.0f, -15.0f, -15.0f);
  glEnd();
  //车下半部 右
  glBegin(GL_POLYGON);
  glColor3f(0.1, 0.1, 0.8);
  glVertex3f(-40.0f, 0.0f, -15.0f);
  glVertex3f(40.0f, 0.0f, -15.0f);
  // glColor3f(1, 0.5, 0.8);
  glVertex3f(40.0f, -15.0f, -15.0f);
  glVertex3f(-40.0f, -15.0f, -15.0f);
  glEnd();
  //车下半部 上
  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(-40.0f, 0.0f, 15.0f);
  glVertex3f(-40.0f, 0.0f, -15.0f);
  glVertex3f(40.0f, 0.0f, -15.0f);
  glVertex3f(40.0f, 0.0f, 15.0f);
  glEnd();
  //车下半部 下
  glBegin(GL_POLYGON);
  glColor3f(0.8, 0.5, 0.2);
  glVertex3f(-40.0f, -15.0f, 15.0f);
  glVertex3f(-40.0f, -15.0f, -15.0f);
  glVertex3f(40.0f, -15.0f, -15.0f);
  glVertex3f(40.0f, -15.0f, 15.0f);
  glEnd();
  //车上半部 左
  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(-20.0f, 0.0f, 15.0f);
  glVertex3f(-10.0f, 10.0f, 15.0f);
  glVertex3f(20.0f, 10.0f, 15.0f);
  glVertex3f(25.0f, 0.0f, 15.0f);
  glEnd();
  //车上半部 右
  glBegin(GL_POLYGON);
  glColor3f(0, 0, 1);
  glVertex3f(-20.0f, 0.0f, -15.0f);
  glVertex3f(-10.0f, 10.0f, -15.0f);
  glVertex3f(20.0f, 10.0f, -15.0f);
  glVertex3f(25.0f, 0.0f, -15.0f);
  glEnd();
  //车上半部 上
  glBegin(GL_POLYGON);
  glColor3f(0.3, 0.2, 0.5);
  glVertex3f(-10.0f, 10.0f, 15.0f);
  glVertex3f(-10.0f, 10.0f, -15.0f);
  glVertex3f(20.0f, 10.0f, -15.0f);
  glVertex3f(20.0f, 10.0f, 15.0f);
  glEnd();
  //车上半部 前
  glBegin(GL_POLYGON);
  glColor3f(0.5, 0.8, 0.8);
  glVertex3f(-10.0f, 10.0f, 15.0f);
  glVertex3f(-20.0f, 0.0f, 15.0f);
  glVertex3f(-20.0f, 0.0f, -15.0f);
  glVertex3f(-10.0f, 10.0f, -15.0f);
  glEnd();
  // //车上半部 后
  glBegin(GL_POLYGON);
  glColor3f(0, 0.5, 0.5);
  glVertex3f(20.0f, 10.0f, 15.0f);
  glVertex3f(20.0f, 10.0f, -15.0f);
  glVertex3f(25.0f, 0.0f, -15.0f);
  glVertex3f(25.0f, 0.0f, 15.0f);
  glEnd();
  //车轮
  float pos[4][3] = {{-20.0f, -15.0f, 15.0f},
                     {-20.0f, -15.0f, -15.0f},
                     {25.0f, -15.0f, 15.0f},
                     {25.0f, -15.0f, -15.0f}};
  for (int i = 0; i < 4; ++i) {
    glPushMatrix();
    glTranslated(pos[i][0], pos[i][1], pos[i][2]);
    if (i < 2) glRotatef(myCar.tireAngle, 0.0f, 1.0f, 0.0f);
    glColor3f(0, 0, 0);
    glutSolidTorus(2, 5, 5, 100);
    glColor3f(0.2, 0.2, 0.2);
    glutSolidTorus(2, 2, 5, 100);
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
  cpoint poly[9] = {{0, 0},       {0, 1000},    {500, 1500},
                    {1500, 1500}, {2000, 1000}, {2000, 0},
                    {1500, -500}, {500, -500},  {0, 0}};
  cpoint p[4] = {{myCar.y + myCar.WIDTH, -myCar.x - myCar.WIDTH},
                 {myCar.y + myCar.WIDTH, -myCar.x + myCar.WIDTH},
                 {myCar.y - myCar.WIDTH, -myCar.x + myCar.WIDTH},
                 {myCar.y - myCar.WIDTH, -myCar.x - myCar.WIDTH}};
  glColor3f(1.0f, 1.0f, 1.0f);
  for (int i = 0; i < 4; ++i) {
    if (!complInsideConvex(p[i], poly, 8)) {
      glColor3f(1.0f, 0.0f, 0.0f);
      break;
    }
  }
  drawPolygon(poly, 8);
}

void renderScene(void) {
  // Clear Color and Depth Buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Reset transformations
  glLoadIdentity();
  // Set the camera
  float distance = 200 + myCar.vx * 6;
  gluLookAt(-myCar.y + distance * sin(myCar.radian - myCar.delta), y,
            -myCar.x + distance * cos(myCar.radian - myCar.delta), -myCar.y,
            22.0f, -myCar.x, 0.0f, 1.0f, 0.0f);

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
  // init GLUT and create window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(800, 600);
  glutCreateWindow("My 3D Car");

  // register callbacks
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutIdleFunc(renderScene);

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