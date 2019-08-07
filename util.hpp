#ifndef UTIL_HPP
#define UTIL_HPP

#include <cmath>
#include <vector>

#include "point.h"

using namespace std;

class Util {
 public:
  // check whether two lines are intersectant
  static bool isLineIntersect(const Point &p1, const Point &p2, const Point &p3,
                              const Point &p4) {
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
  static bool isPloygonIntersect(const vector<Point> p1,
                                 const vector<Point> p2) {
    int n1 = p1.size(), n2 = p2.size();
    for (int i = 0; i < n1; ++i)
      for (int j = 0; j < n2; ++j)
        if (isLineIntersect(p1[i], p1[(i + 1) % n1], p2[j], p2[(j + 1) % n2]))
          return true;
    return false;
  }

  static float distanceFromP2L(const Point &p0, const Point &p1,
                                const Point &p2) {
    Point v1(p2.x - p1.x, p2.y - p1.y), v2(p0.x - p1.x, p0.y - p1.y),
        v3(p0.x - p2.x, p0.y - p2.y);
    if (v1.x * v2.x + v1.y * v2.y < 0)
      return sqrt(v2.x * v2.x + v2.y * v2.y);
    else if (v1.x * v3.x + v1.y * v3.y > 0)
      return sqrt(v3.x * v3.x + v3.y * v3.y);
    else
      return fabs(v1.x * v2.y - v2.x * v1.y) / sqrt(v1.x * v1.x + v1.y * v1.y);
  }

  static bool pointInsideConvex(const Point &p, vector<Point> con) {
    int n = con.size(), j = n - 1, res = 0;
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
};
#endif /* UTIL_HPP */
