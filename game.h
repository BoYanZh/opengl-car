#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <string>
#include <vector>

#include "car.h"
#include "point.h"

using namespace std;

class Game {
 public:
  static Game *Instance(string _mapName = "map.txt");
  void moveCar();
  void refresh();
  void changeCamera();
  void setSize(int _w, int _h);
  bool keyStatus[256];
  bool specialStatus[256];

 private:
  static Game *m_pInstance;
  Car car;
  Game(string _mapName);
  Game(Game const &);
  Game &operator=(Game const &);

  enum gamestatus { WAIT, STARTED, FINISHED, SUCCEED, FAIL };
  int gameStatus = WAIT;
  int w, h;
  int currentPL = -1;
  int waitCount = 120;
  int frame;
  long time, timeBase, timeStart, timeDelta;
  float bestTime = 0.0f;
  bool isHit = false;
  string mapName = "map.txt";
  vector<Point> outer, inner;
  vector<vector<Point>> parkingLots;
  vector<Point> startLine;
  vector<Point> finishLine;
  void init();
  // draw the green ground of parking lots
  void drawParkingLots();
  // draw the start line
  void drawStartLine();
  void drawFinishLine();
  // draw a line
  void drawLine(const Point &p1, const Point &p2, const float height = 1.0f);
  // draw a vertical stick
  void drawStick(const Point &p);
  void drawCircle(const Point point, const float height = 1.0f);
  // draw a polygon
  void drawPolygon(const vector<Point> poly, const float height = 1.0f,
                   const bool withStick = true);
  // draw border line of the track
  void drawBorder();
  // draw the ground
  void drawGround();
  void drawText();
  void setOrthographicProjection();
  void restorePerspectiveProjection();
  void renderBitmapString(float x, float y, float z, void *font, char *string);
  void checkStatus();
};
#endif /* GAME_H */
