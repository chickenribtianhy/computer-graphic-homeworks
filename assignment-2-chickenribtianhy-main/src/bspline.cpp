#include <bspline.h>
#include <utils.h>
#include <vector>
#include <fstream>
#include <sstream>

#define FREQ 50

BsplineCurve::BsplineCurve(int m, int degree)
{
  control_points_.resize(m);
  _degree = degree;
  for (int i = 0; i <= _degree; i++)
    _knots.push_back(0.0f);
  for (int i = _degree + 1; i < m; i++)
    _knots.push_back((float)(i - _degree) / (m - _degree + 1));
  for (int i = m; i <= m + _degree; i++)
    _knots.push_back(1.0f + 1e-5);
}

BsplineCurve::BsplineCurve(std::vector<vec3> &control_points, int degree)
{
  int m = control_points.size();
  control_points_ = control_points;
  _degree = degree;
  for (int i = 0; i <= _degree; i++)
    _knots.push_back(0.0f);
  for (int i = _degree + 1; i < m; i++)
    _knots.push_back((float)(i - _degree) / (m - _degree + 1));
  for (int i = m; i <= m + _degree; i++)
    _knots.push_back(1.0f + 1e-5);
}

void BsplineCurve::setControlPoint(int i, vec3 point)
{
  control_points_[i] = point;
}

int BsplineCurve::find_dist(float t)
{
  int _counter = 0;
  for (auto knot : _knots)
  {
    if (knot > t)
    {
      return _counter - 1;
    }
    ++_counter;
  }
}

/**
 * TODO: evaluate the point at t with the given control_points
 */
Vertex BsplineCurve::evaluate(std::vector<vec3> &control_points, float t)
{
  std::vector<vec3> _copy;
  int k = find_dist(t);
  /*
  00 01 02 03
  10 11 12
  20 21
  30
   */
  for (int i = 0; i <= _degree; ++i)
  {
    _copy.push_back(control_points[k - _degree + i]);
  }
  // _copy <- control_points
  Vertex _vertex;
  // float _t_ = 1.0 - t;
  for (int i = 1; i < _degree + 1; ++i)
  {
    if (i == _degree)
    {
      _vertex.normal = glm::normalize(_copy[i] - _copy[i - 1]);
    }
    for (int j = _degree; j >= i; --j)
    {
      float _t_;
      _t_ = t - _knots[j + k - _degree];
      if (_knots[j + 1 + k - i] - _knots[j + k - _degree] == 0)
        _t_ = 0; // define 0/0=0
      else
        _t_ = _t_ / (_knots[j + 1 + k - i] - _knots[j + k - _degree]);
      _copy[j] = (1 - _t_) * _copy[j - 1] + _t_ * _copy[j];
    }
  }

  _vertex.position = _copy[_degree];
  return _vertex;
}

Vertex BsplineCurve::evaluate(float t)
{
  return evaluate(control_points_, t);
}

/**
 * TODO: generate an Object of the current Bezier curve
 */
// Object BsplineCurve::generateObject()
// {
//   Object _object;
//   for (int i = 0; i < FREQ; ++i)
//   {
//     _object.vertices.push_back(evaluate((float)i / FREQ));
//     _object.indices.push_back(i);
//   }
//   return _object;
// }

BsplineSurface::BsplineSurface(int m, int n, int degree)
{
  _degree = degree;
  control_points_m_.resize(m);
  for (auto &sub_vec : control_points_m_)
  {
    sub_vec.resize(n);
  }
  control_points_n_.resize(n);
  for (auto &sub_vec : control_points_n_)
  {
    sub_vec.resize(m);
  }
}

/**
 * @param[in] i: index (i < m)
 * @param[in] j: index (j < n)
 * @param[in] point: the control point with index i, j
 */
void BsplineSurface::setControlPoint(int i, int j, vec3 point)
{
  control_points_m_[i][j] = point;
  control_points_n_[j][i] = point;
}

Vertex BsplineSurface::evaluate(
    float u, float v)
{
  Vertex _vertex;
  std::vector<vec3> _direction_0;
  for (auto _one_of_controlpoints : control_points_m_)
    _direction_0.push_back(BsplineCurve(_one_of_controlpoints, _degree).evaluate(u).position);
  _vertex = BsplineCurve(_direction_0, _degree).evaluate(v);
  // meanwhile _vertex.normal is tangent along v
  Vertex _vertex_0;
  std::vector<vec3> _direction_1;
  for (auto _one_of_controlpoints : control_points_n_)
    _direction_1.push_back(BsplineCurve(_one_of_controlpoints, _degree).evaluate(v).position);
  _vertex_0 = BsplineCurve(_direction_1, _degree).evaluate(u);
  // meanwhile _vertex_0.normal is tangent along u
  // assert(_vertex.position == _vertex_0.position)
  _vertex.normal = glm::normalize(glm::cross(_vertex.normal, _vertex_0.normal));
  return _vertex;
}

/**
 * TODO: generate an Object of the current Bezier surface
 */
Object BsplineSurface::generateObject()
{
  Object _object;
  float rate = 1.0 / FREQ;
  int counter = 0;
  for (int i = 0; i < FREQ; ++i)
  {
    for (int j = 0; j < FREQ; ++j)
    {
      float u = i * rate;
      float v = j * rate;
      _object.vertices.push_back(evaluate(u, v));               // 1
      _object.vertices.push_back(evaluate(u + rate, v));        // 2
      _object.vertices.push_back(evaluate(u + rate, v + rate)); // 4
      _object.vertices.push_back(evaluate(u, v));               // 1
      _object.vertices.push_back(evaluate(u, v + rate));        // 3
      _object.vertices.push_back(evaluate(u + rate, v + rate)); // 4
      for (int _k = 0; _k < 6; ++_k)
        _object.indices.push_back(counter++);
      // std::cout << "counter = " << counter << std::endl;
    }
  }
  return _object;
}

/**
 * TODO: read in bzs file to generate a vector of Bezier surface
 * for the first line we have b p m n
 * b means b bezier surfaces, p means p control points.
 *
 */
std::vector<BsplineSurface> read_bspline(const std::string &path)
{
  int b, p, m, n;
  std::ifstream infile(path);
  std::string _buffer;
  getline(infile, _buffer);
  std::stringstream(_buffer) >> b >> p >> m >> n;
  // b surfaces
  std::vector<std::vector<int>> ctr_points(b);
  for (int i = 0; i < b; ++i)
  {
    getline(infile, _buffer);
    int _temp;
    std::stringstream _buffer_stream(_buffer);
    while (_buffer_stream >> _temp)
      ctr_points[i].push_back(_temp);
  }
  // p control points
  std::vector<vec3> point_pos(p);
  for (int i = 0; i < p; ++i)
  {
    getline(infile, _buffer);
    std::stringstream(_buffer) >> point_pos[i].x >> point_pos[i].y >> point_pos[i].z;
  }

  std::vector<BsplineSurface> _surfaces;
  for (int k = 0; k < b; ++k)
  {
    BsplineSurface _beziersurface(m, n, 3);
    for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j)
        _beziersurface.setControlPoint(i, j, point_pos[ctr_points[k][i * n + j]]);
    _surfaces.push_back(_beziersurface);
  }
  return _surfaces;
}