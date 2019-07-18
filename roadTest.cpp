#include <glut.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
using namespace std;

bool keyStatus[256];
bool specialStatus[256];
int const FPS = 60;
float const PI = 3.14159, eps = 1e-4;
double const MOVE_SPEED = 20, TURN_SPEED = 0.4, ACC = 0.1;
const float WIDTH = 60, LENGTH = 140, HEIGHT = 40;

struct car {
  double x = 200;
  double y = 200;
  double z = 0;
  double radian = PI / 2;
  double tiltRadian = 0;
  double tireAngle = 0;
  double vx = 0;
  double vy = 0;
  double delta = 0;
  int staticCount = 0;
};

struct point2d {
  double x;
  double y;
};

struct point3d {
  double x;
  double y;
  double z;
};

struct road {
  point3d p1;
  point3d p2;
  point3d p3;
};

car Car;
road Roads[] = {{{0, 0, 200}, {0, 1000, 200}, {1000, 1000, 200}},
                {{0, 0, 200}, {1000, 0, 200}, {1000, 1000, 200}},
                {{0, 1000, 200}, {0, 2000, 0}, {1000, 2000, 0}},
                {{0, 1000, 200}, {1000, 1000, 200}, {1000, 2000, 0}},
                {{1000, 0, 200}, {1000, 1000, 200}, {2000, 0, 0}},
                {{2000, 1000, 0}, {1000, 1000, 200}, {2000, 0, 0}},
                {{1000, 1000, 200}, {2000, 1000, 0}, {1000, 2000, 0}}};

float y = HEIGHT * 6;

bool complInsideConvex(const point2d &p, point2d *con, int n) {
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
  return (res);
}

bool intersection(const point2d &p1, const point2d &p2, const point2d &p3,
                  const point2d &p4) {
  //快速排斥实验
  if ((p1.x > p2.x ? p1.x : p2.x) < (p3.x < p4.x ? p3.x : p4.x) ||
      (p1.y > p2.y ? p1.y : p2.y) < (p3.y < p4.y ? p3.y : p4.y) ||
      (p3.x > p4.x ? p3.x : p4.x) < (p1.x < p2.x ? p1.x : p2.x) ||
      (p3.y > p4.y ? p3.y : p4.y) < (p1.y < p2.y ? p1.y : p2.y))
    return false;
  //跨立实验
  if ((((p1.x - p3.x) * (p4.y - p3.y) - (p1.y - p3.y) * (p4.x - p3.x)) *
       ((p2.x - p3.x) * (p4.y - p3.y) - (p2.y - p3.y) * (p4.x - p3.x))) > 0 ||
      (((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) *
       ((p4.x - p1.x) * (p2.y - p1.y) - (p4.y - p1.y) * (p2.x - p1.x))) > 0)
    return false;
  return true;
}

bool polysIntersection(const point2d p1[], int n1, const point2d p2[], int n2) {
  for (int i = 0; i < n1; ++i)
    for (int j = 0; j < n2; ++j)
      if (intersection(p1[i], p1[(i + 1) % n1], p2[j], p2[(j + 1) % n2]))
        return true;
  return false;
}

void Initialization() {
  GLfloat lightSpecular[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat lightPosition[] = {0.5, 0.5, 4.0, 0.0};

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //指定混合函数
  glShadeModel(GL_SMOOTH);

  glMaterialfv(GL_FRONT, GL_SPECULAR, lightSpecular);
  glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

  glEnable(GL_BLEND);  //启用混合状态
  glEnable(GL_LIGHTING);        //启用光照
  glEnable(GL_LIGHT0);          //打开光源0
  glEnable(GL_DEPTH_TEST);      //启用深度检测
  glEnable(GL_COLOR_MATERIAL);  //材质跟踪当前绘图色
  glEnable(GLUT_MULTISAMPLE);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glEnable(GL_SMOOTH);
}

void changeSize(int w, int h) {
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
  Initialization();
}

void drawLine(const point2d &p1, const point2d &p2) {
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

void drawStick(const point2d &p) {
  for (int j = 0; j < 40; ++j) {
    glBegin(GL_POLYGON);
    // glColor4f(0.0f, 0.0f, 0.0f, 0.5);
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

void drawPolygon(const point2d poly[], const int n,
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

void drawOldCar() {
  // cout << Car.radian * 180 / PI << ' ' << Car.y << ' ' << Car.x <<
  // endl;
  glPushMatrix();
  // glRotatef(Car.radian * 180 / PI, 0.0f, 1.0f, 0.0f);
  // glRotatef(90, 0.0f, 1.0f, 0.0f);
  glRotatef(90, 1.0f, 1.0f, 0.0f);
  double alpha = Car.y == 0 ? PI / 2 : atan(Car.x / Car.y);
  double r1 = sqrt(Car.x * Car.x + Car.y * Car.y);
  if (Car.y < 0)
    glTranslatef(r1 * cos(alpha + Car.radian), HEIGHT * 1.5 + 10,
                 r1 * sin(alpha + Car.radian));
  else
    glTranslatef(-r1 * cos(alpha + Car.radian), HEIGHT * 1.5 + 10,
                 -r1 * sin(alpha + Car.radian));
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
    if (i < 2) glRotatef(Car.tireAngle, 0.0f, 1.0f, 0.0f);
    glColor3f(0, 0, 0);
    glutSolidTorus(10, HEIGHT / 2, 5, 100);
    glColor3f(0.2, 0.2, 0.2);
    glutSolidTorus(10, 10, 5, 100);
    glPopMatrix();
  }
  glPopMatrix();
}

void drawCar() {
  glPushMatrix();
  double alpha =
      abs(Car.x) < eps ? (-2 * (Car.y >= 0) + 3) * PI / 2 : atan(Car.y / Car.x);
  double r1 = sqrt(Car.x * Car.x + Car.y * Car.y);
  glRotatef(Car.radian / PI * 180, 0.0f, 0.0f, 1.0f);
  glTranslatef(LENGTH / 5 * 3, 0.0f, 0.0f);
  if (Car.x >= 0) {
    glTranslatef(r1 * cos(-Car.radian + alpha), r1 * sin(-Car.radian + alpha),
                 0.0f);
  } else {
    glTranslatef(r1 * cos(PI - Car.radian + alpha),
                 r1 * sin(PI - Car.radian + alpha), 0.0f);
  }
  glTranslatef(0.0f, 0.0f, Car.z);
  glRotatef(Car.tiltRadian / PI * 180, 0.0f, 1.0f, 0.0f);
  {
    //车下半部 下
    glBegin(GL_POLYGON);
    glColor3f(0.8, 0.5, 0.2);
    glVertex3f(LENGTH, -WIDTH, HEIGHT);
    glVertex3f(-LENGTH, -WIDTH, HEIGHT);
    glVertex3f(-LENGTH, WIDTH, HEIGHT);
    glVertex3f(LENGTH, WIDTH, HEIGHT);
    glEnd();
    //车下半部 上
    glBegin(GL_POLYGON);
    glColor3f(0, 0, 1);
    glVertex3f(LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, 2 * HEIGHT);
    glEnd();
    //车下半部 左
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(-LENGTH, -WIDTH, HEIGHT);
    glVertex3f(-LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(-LENGTH, WIDTH, HEIGHT);
    glEnd();
    //车下半部 右
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(LENGTH, -WIDTH, HEIGHT);
    glVertex3f(LENGTH, -WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, HEIGHT);
    glEnd();
    //车下半部 前
    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(-LENGTH, WIDTH, HEIGHT);
    glVertex3f(-LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, 2 * HEIGHT);
    glVertex3f(LENGTH, WIDTH, HEIGHT);
    glEnd();
    //车下半部 后
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

void carPos(point2d re[]) {
  float r, r1, delta;
  r = sqrt(WIDTH * WIDTH + LENGTH * LENGTH);
  r1 = LENGTH / 5 * 3;
  delta = atan(WIDTH / LENGTH);
  float angle[] = {delta, PI - delta, PI + delta, 2 * PI - delta};
  for (int i = 0; i < 4; ++i)
    re[i] = {Car.x + r1 * cos(Car.radian) + r * cos(Car.radian + angle[i]),
             Car.y + r1 * sin(Car.radian) + r * sin(Car.radian + angle[i])};
}

void drawSigns() {
  point2d p[4];
  carPos(p);
  glColor3f(0.5f, 0.5f, 0.5f);
  drawPolygon(p, 4, false);
}

void drawGround() {
  // Draw ground
  glBegin(GL_QUADS);
  glColor3f(0.3f, 0.3f, 0.3f);
  glVertex3f(-10000.0f, -10000.0f, -1.0f);
  glVertex3f(-10000.0f, 10000.0f, -1.0f);
  glVertex3f(10000.0f, 10000.0f, -1.0f);
  glVertex3f(10000.0f, -10000.0f, -1.0f);
  glEnd();
}

void drawRoad() {
  for (auto &road : Roads) {
    glBegin(GL_POLYGON);
    glColor3f(0.7f, 0.7f, 0.7f);
    glVertex3f(road.p1.x, road.p1.y, road.p1.z);
    glVertex3f(road.p2.x, road.p2.y, road.p2.z);
    glVertex3f(road.p3.x, road.p3.y, road.p3.z);
    glEnd();
  }
}

void drawScene() {
  drawSigns();
  drawRoad();
  drawGround();
}

void renderScene(void) {
  // Clear Color and Depth Buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Reset transformations
  glLoadIdentity();
  // Set the camera
  float distance = WIDTH * 8;  // + (Car.vx > 0 ? Car.vx * WIDTH / 4 : 0);
  gluLookAt(Car.x - distance * cos(Car.radian - Car.delta),
            Car.y - distance * sin(Car.radian - Car.delta), Car.z + y, Car.x,
            Car.y, Car.z, 0.0f, 0.0f, 1.0f);
  drawCar();
  drawScene();
  glutSwapBuffers();
}

void keyPressed(unsigned char key, int xx, int yy) {
  if (key == 27) exit(0);
  keyStatus[key] = true;
}

void keyUp(unsigned char key, int x, int y) { keyStatus[key] = false; }

void specialPressed(int key, int x, int y) { specialStatus[key] = true; }

void specialUp(int key, int x, int y) { specialStatus[key] = false; }

void checkTurn() {
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

void solveFunc(double a, double b, double c, double d, double e, double f,
               double re[]) {
  if (a * e == b * d)
    cout << "no solution";
  else {
    re[0] = (b * f - e * c) / (a * e - b * d);
    re[1] = (a * f - d * c) / (b * d - a * e);
  }
}

void moveCar() {
  Car.z = Car.tiltRadian = 0;
  for (auto &road : Roads) {
    point2d tmpRoad[] = {
        {road.p1.x, road.p1.y}, {road.p2.x, road.p2.y}, {road.p3.x, road.p3.y}};
    if (complInsideConvex({Car.x, Car.y}, tmpRoad, 3)) {
      double v1[] = {road.p2.x - road.p1.x, road.p2.y - road.p1.y,
                     road.p2.z - road.p1.z},
             v2[] = {road.p3.x - road.p2.x, road.p3.y - road.p2.y,
                     road.p3.z - road.p2.z},
             v3[] = {Car.x - road.p1.x, Car.y - road.p1.y, Car.z - road.p1.z};
      double re[2] = {0, 0};
      solveFunc(v1[0], v2[0], -v3[0], v1[1], v2[1], -v3[1], re);
      Car.z = road.p1.z + re[0] * v1[2] + re[1] * v2[2];
      Car.tiltRadian = atan((re[0] * v1[2] + re[1] * v2[2]) /
                            sqrt(v3[0] * v3[0] + v3[1] * v3[1]));
      break;
    }
  }
  int dire = 0;
  if (specialStatus[GLUT_KEY_UP] && !specialStatus[GLUT_KEY_DOWN])
    dire = 1;
  else if (specialStatus[GLUT_KEY_DOWN] && !specialStatus[GLUT_KEY_UP])
    dire = -1;
  if (dire) {
    if (dire * Car.vx >= 0 && !Car.staticCount) {  // speed up
      Car.vx += dire * (-2 / 3 * abs(Car.vx) + ACC) * 2;
    } else if (dire * Car.vx < 0) {  // break down
      if (Car.vx > 0)
        Car.vx = max(0.0, Car.vx - (-2 / 3 * abs(Car.vx) + ACC) * 4);
      if (Car.vx < 0)
        Car.vx = min(0.0, Car.vx + (-2 / 3 * abs(Car.vx) + ACC) * 4);
      if (abs(Car.vx) < 1e4) Car.staticCount = 15;
      // Car.vx += dire * (-2 / 3 * abs(Car.vx) + ACC) * 4;
    }
    Car.vx = max(min(MOVE_SPEED, Car.vx), -MOVE_SPEED / 2);
  } else {
    if (Car.vx > 0) Car.vx = max(0.0, Car.vx - ACC);
    if (Car.vx < 0) Car.vx = min(0.0, Car.vx + ACC);
  }
  if (Car.staticCount) --Car.staticCount;
  if (Car.delta > 0) Car.delta = fmax(0.0, Car.delta - 0.015f);
  if (Car.delta < 0) Car.delta = fmin(0.0, Car.delta + 0.015f);
  if (Car.vx) checkTurn();
  Car.x += cos(Car.radian) * Car.vx;
  Car.y += sin(Car.radian) * Car.vx;
}

void timerFunc(int value) {
  moveCar();
  if (keyStatus[(int)'w']) y += 5.0f;
  if (keyStatus[(int)'s']) y -= 5.0f;
  if (keyStatus[32]) Car.z += 5.0f;
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

  // opengl init
  Initialization();

  // register callbacks
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  // glutIdleFunc(renderScene);

  // glutIgnoreKeyRepeat(1);
  glutKeyboardFunc(keyPressed);
  glutKeyboardUpFunc(keyUp);
  glutSpecialFunc(specialPressed);
  glutSpecialUpFunc(specialUp);
  // Timer init
  glutTimerFunc(1000 / FPS, timerFunc, 1);

  // enter GLUT event processing cycle
  glutMainLoop();

  return 1;
}