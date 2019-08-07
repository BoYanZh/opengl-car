#include <fstream>
#include <vector>

#include "car.h"
#include "game.h"
#include "point.h"
#include "util.hpp"

#include "main.h"

Game::Game(Game const &) {}
Game &Game::operator=(Game const &) { return *this; }
Game *Game::Instance(string _mapName) {
  static Game *m_pInstance = nullptr;
  if (!m_pInstance) m_pInstance = new Game(_mapName);
  return m_pInstance;
}

void Game::init() {
  car.reset();
  timeStart = glutGet(GLUT_ELAPSED_TIME);
  currentPL = -1;
  waitCount = 120;
  gameStatus = WAIT;
  isHit = false;
}

void Game::moveCar() {
  if (gameStatus != FAIL) car.move(specialStatus);
  if (gameStatus == FAIL || gameStatus == SUCCEED) {
    if (gameStatus == FAIL) {
      if (waitCount > 60) {
        car.moveCamera(10.0f);
      }
    }
    if (--waitCount == 0) {
      init();
    }
  }
}

void Game::setSize(int _w, int _h) { w = _w, h = _h; }

void Game::restorePerspectiveProjection() {
  glMatrixMode(GL_PROJECTION);
  // restore previous projection matrix
  glPopMatrix();

  // get back to modelview mode
  glMatrixMode(GL_MODELVIEW);
}

void Game::setOrthographicProjection() {
  // switch to projection mode
  glMatrixMode(GL_PROJECTION);
  // save previous matrix which contains the
  // settings for the perspective projection
  glPushMatrix();
  // reset matrix
  glLoadIdentity();
  // set a 2D orthographic projection
  gluOrtho2D(0, w, h, 0);
  // switch back to modelview mode
  glMatrixMode(GL_MODELVIEW);
}

void Game::renderBitmapString(float x, float y, float z, void *font,
                              char *string) {
  char *c;
  glRasterPos3f(x, y, z);
  for (c = string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

void Game::drawText() {
  static char s1[60], s2[60], s3[60];
  frame++;
  time = glutGet(GLUT_ELAPSED_TIME);
  if (time - timeBase > 1000) {
    sprintf(s1, "FPS: %4.2f", frame * 1000.0 / (time - timeBase));
    timeBase = time;
    frame = 0;
  }
  switch (gameStatus) {
    case STARTED:
    case FINISHED:
      timeDelta = time - timeStart;
      sprintf(s2, "Time: %4.2fs", timeDelta / 1000.0f);
      break;
    case WAIT:
      sprintf(s2, "Time: %4.2fs", 0.0f);
      break;
    case SUCCEED:
      sprintf(s2, "Time: %4.2fs", timeDelta / 1000.0f);
      if (bestTime == 0 || timeDelta / 1000.0f < bestTime) {
        bestTime = timeDelta / 1000.0f;
        fstream output(mapName, fstream::app);
        output << bestTime << endl;
        output.close();
      }
  }
  if (bestTime != 0)
    sprintf(s3, "Best Time: %4.2fs", bestTime);
  else
    sprintf(s3, "Best Time: NaN");
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
  setOrthographicProjection();
  void *font = GLUT_BITMAP_8_BY_13;
  glPushMatrix();
  glLoadIdentity();
  renderBitmapString(10, 20, 0, font, s1);
  renderBitmapString(10, 35, 0, font, s2);
  renderBitmapString(10, 50, 0, font, s3);
  glPopMatrix();
  restorePerspectiveProjection();
}

void Game::refresh() {
  checkStatus();
  car.draw();
  drawBorder();
  if (gameStatus != SUCCEED) {
    drawStartLine();
    drawFinishLine();
    drawParkingLots();
  }
  drawGround();
  drawText();
}

void Game::changeCamera() {
  if (keyStatus[(int)'w']) car.moveCamera(5.0);
  if (keyStatus[(int)'s']) car.moveCamera(-5.0);
}

void Game::checkStatus() {
  if (gameStatus != SUCCEED && isHit) {
    gameStatus = FAIL;
    return;
  }
  vector<Point> p = car.getPosition();
  if (gameStatus == WAIT && Util::isPloygonIntersect(p, startLine)) {
    gameStatus = STARTED;
    timeStart = glutGet(GLUT_ELAPSED_TIME);
    ++currentPL;
    return;
  }
  if (gameStatus == FINISHED && Util::isPloygonIntersect(p, finishLine)) {
    gameStatus = SUCCEED;
    return;
  }
  if (gameStatus == STARTED) {
    if (parkingLots.size() == 0)
      gameStatus = FINISHED;
    else if (Util::pointInsideConvex(p[0], parkingLots[currentPL]) &&
             Util::pointInsideConvex(p[1], parkingLots[currentPL]) &&
             Util::pointInsideConvex(p[2], parkingLots[currentPL]) &&
             Util::pointInsideConvex(p[3], parkingLots[currentPL])) {
      ++currentPL;
      if (currentPL == (int)parkingLots.size()) gameStatus = FINISHED;
    }
  }
}

// draw the green ground of parking lots
void Game::drawParkingLots() {
  int count = 0;
  for (auto const &pl : parkingLots) {
    glBegin(GL_QUADS);
    currentPL == count ? glColor4f(1.0f, 0.0f, 0.0f, 0.5f)
                       : glColor4f(0.3f, 1.0f, 0.3f, 0.5f);
    glVertex3f(pl[0].x, pl[0].y, 1.5f);
    glVertex3f(pl[1].x, pl[1].y, 1.5f);
    glVertex3f(pl[2].x, pl[2].y, 1.5f);
    glVertex3f(pl[3].x, pl[3].y, 1.5f);
    glEnd();
    ++count;
  }
}

// draw the start line
void Game::drawStartLine() {
  gameStatus == WAIT ? glColor4f(1.0f, 0.0f, 0.0f, 0.5f)
                     : glColor4f(0.3f, 1.0f, 0.3f, 0.5f);
  glBegin(GL_QUADS);
  glVertex3f(startLine[0].x, startLine[0].y, 0);
  glVertex3f(startLine[0].x, startLine[0].y, 200);
  glVertex3f(startLine[1].x, startLine[1].y, 200);
  glVertex3f(startLine[1].x, startLine[1].y, 0);
  glEnd();
}

// draw the finish line
void Game::drawFinishLine() {
  gameStatus == FINISHED ? glColor4f(1.0f, 0.0f, 0.0f, 0.5f)
                         : glColor4f(0.3f, 1.0f, 0.3f, 0.5f);
  glBegin(GL_QUADS);
  glVertex3f(finishLine[0].x, finishLine[0].y, 0);
  glVertex3f(finishLine[0].x, finishLine[0].y, 200);
  glVertex3f(finishLine[1].x, finishLine[1].y, 200);
  glVertex3f(finishLine[1].x, finishLine[1].y, 0);
  glEnd();
}

// draw a line
void Game::drawLine(const Point &p1, const Point &p2, const float height) {
  float alpha =
      abs(p1.y - p2.y) < eps ? PI / 2 : atan((p1.x - p2.x) / (p1.y - p2.y));
  float const w = 10;
  glBegin(GL_QUADS);
  glVertex3f((p1.x - w * cos(alpha)), p1.y + w * sin(alpha), height);
  glVertex3f((p2.x - w * cos(alpha)), p2.y + w * sin(alpha), height);
  glVertex3f((p2.x + w * cos(alpha)), p2.y - w * sin(alpha), height);
  glVertex3f((p1.x + w * cos(alpha)), p1.y - w * sin(alpha), height);
  glEnd();
}

// draw a vertical stick
void Game::drawStick(const Point &p) {
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

void Game::drawCircle(const Point point, const float height) {
  glBegin(GL_POLYGON);
  for (int j = 0; j < 20; ++j) {
    glVertex3f(point.x + 10 * cos(2 * PI / 20 * j),
               point.y + 10 * sin(2 * PI / 20 * j), height);
  }
  glEnd();
}

// draw a polygon
void Game::drawPolygon(const vector<Point> poly, const float height,
                       const bool withStick) {
  int n = poly.size();
  if (!n) return;
  for (int i = 0; i < n - 1; ++i) {
    drawLine(poly[i], poly[i + 1], height);
    if (withStick) drawStick(poly[i]);
    drawCircle(poly[i], height);
  }
  drawLine(poly[n - 1], poly[0], height);
  if (withStick) drawStick(poly[n - 1]);
  drawCircle(poly[n - 1], height);
}

// draw border line of the track
void Game::drawBorder() {
  vector<Point> p = car.getPosition();
  bool hitOuter = Util::isPloygonIntersect(p, outer),
       hitInner = Util::isPloygonIntersect(p, inner);
  for (auto const &sp : outer) {
    if (hitOuter) break;
    if (Util::distanceFromP2L(sp, p[0], p[1]) <= 10 ||
        Util::distanceFromP2L(sp, p[1], p[2]) <= 10 ||
        Util::distanceFromP2L(sp, p[2], p[3]) <= 10 ||
        Util::distanceFromP2L(sp, p[3], p[0]) <= 10)
      hitOuter = true;
  }
  for (auto const &sp : inner) {
    if (hitInner) break;
    if (Util::distanceFromP2L(sp, p[0], p[1]) <= 10 ||
        Util::distanceFromP2L(sp, p[1], p[2]) <= 10 ||
        Util::distanceFromP2L(sp, p[2], p[3]) <= 10 ||
        Util::distanceFromP2L(sp, p[3], p[0]) <= 10)
      hitInner = true;
  }
  glColor3f(0.5f, 0.5f, 0.5f);
  drawPolygon(p, 0.5f, false);
  hitOuter ? glColor3f(1.0f, 0.0f, 0.0f) : glColor3f(1.0f, 1.0f, 1.0f);
  drawPolygon(outer, 0.25f);
  hitInner ? glColor3f(1.0f, 0.0f, 0.0f) : glColor3f(1.0f, 1.0f, 1.0f);
  drawPolygon(inner, 0.25f);
  isHit = hitOuter || hitInner;
}

// draw the ground
void Game::drawGround() {
  glBegin(GL_QUADS);
  glColor3f(0.3f, 0.3f, 0.3f);
  glVertex3f(-10000.0f, -10000.0f, -2.0f);
  glVertex3f(-10000.0f, 10000.0f, -2.0f);
  glVertex3f(10000.0f, 10000.0f, -2.0f);
  glVertex3f(10000.0f, -10000.0f, -2.0f);
  glEnd();
}

Game::Game(string _mapName)
    : car(), timeBase(glutGet(GLUT_ELAPSED_TIME)), mapName(_mapName) {
  int n;
  float x, y;
  vector<Point> tmpPoint;
  ifstream input(mapName);
  input >> n;
  for (int i = 0; i != n; ++i) {
    input >> x >> y;
    outer.push_back({x, y});
  }
  input >> n;
  for (int i = 0; i != n; ++i) {
    input >> x >> y;
    inner.push_back({x, y});
  }
  input >> n;
  for (int i = 0; i != n; ++i) {
    tmpPoint.clear();
    for (int j = 0; j != 4; ++j) {
      input >> x >> y;
      tmpPoint.push_back({x, y});
    }
    parkingLots.push_back(tmpPoint);
  }
  input >> x >> y;
  startLine.push_back({x, y});
  input >> x >> y;
  startLine.push_back({x, y});
  input >> x >> y;
  finishLine.push_back({x, y});
  input >> x >> y;
  finishLine.push_back({x, y});
  while (input >> x) bestTime = x;
  input.close();
}
