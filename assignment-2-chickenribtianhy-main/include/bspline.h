#ifndef _Bspline_H_
#define _Bspline_H_
#include "defines.h"
#include <object.h>

#include <vector>

class BsplineCurve
{
public:
  std::vector<vec3> control_points_;
  std::vector<float> _knots;
  int _degree;

  BsplineCurve(int m, int degree);
  BsplineCurve(std::vector<vec3> &control_points, int degree);

  void setControlPoint(int i, vec3 point);
  Vertex evaluate(std::vector<vec3> &control_points, float t);
  Vertex evaluate(float t);
  int find_dist(float t);
  Object generateObject();
};

class BsplineSurface
{
public:
  std::vector<std::vector<vec3>> control_points_m_;
  std::vector<std::vector<vec3>> control_points_n_;
  int _degree;

  BsplineSurface(int m, int n, int degree);
  void setControlPoint(int i, int j, vec3 point);
  Vertex evaluate(float u, float v);
  Object generateObject();
};

std::vector<BsplineSurface> read_bspline(const std::string &path);
#endif