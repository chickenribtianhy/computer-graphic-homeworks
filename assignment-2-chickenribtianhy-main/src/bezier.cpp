#include <bezier.h>
#include <utils.h>
#include <vector>
#include <fstream>
#include <sstream>

#define FREQ 50

BezierCurve::BezierCurve(int m) { control_points_.resize(m); }

BezierCurve::BezierCurve(std::vector<vec3> &control_points)
{
  control_points_ = control_points;
}

void BezierCurve::setControlPoint(int i, vec3 point)
{
  control_points_[i] = point;
}

/**
 * TODO: evaluate the point at t with the given control_points
 */
Vertex BezierCurve::evaluate(std::vector<vec3> &control_points, float t)
{
  std::vector<vec3> _copy;
  std::vector<vec3>::iterator _iter = control_points.begin();
  for (; _iter != control_points.end(); ++_iter)
  {
    _copy.push_back(*_iter);
  }
  // _copy <- control_points
  int _n = control_points.size();
  float _t_ = 1.0 - t;
  for (int i = 1; i < _n - 1; ++i)
  {
    for (int j = 0; j < _n - i; ++j)
    {
      _copy[j] = _t_ * _copy[j] + t * _copy[j + 1];
    }
  }
  Vertex _vertex;
  _vertex.normal = glm::normalize(_copy[0] - _copy[1]); // tangent
  _copy[0] = _t_ * _copy[0] + t * _copy[1];
  // position: _copy[0];
  _vertex.position = _copy[0];
  return _vertex;
}

Vertex BezierCurve::evaluate(float t)
{
  return evaluate(control_points_, t);
}

/**
 * TODO: generate an Object of the current Bezier curve
 */
Object BezierCurve::generateObject()
{
  Object _object;
  for (int i = 0; i < FREQ; ++i)
  {
    _object.vertices.push_back(evaluate((float)i / FREQ));
    _object.indices.push_back(i);
  }
  return _object;
}

BezierSurface::BezierSurface(int m, int n)
{
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
void BezierSurface::setControlPoint(int i, int j, vec3 point)
{
  control_points_m_[i][j] = point;
  control_points_n_[j][i] = point;
}

/**
 * TODO: evaluate the point at (u, v) with the given control points
 */
Vertex BezierSurface::evaluate(std::vector<std::vector<vec3>> &control_points,
                               float u, float v)
{
  int m = control_points.size();
  int n = control_points[0].size();
  for (int i = 0; i < m; ++i)
    for (int j = 0; j < n; ++j)
      setControlPoint(i, j, control_points[i][j]);
  Vertex _vertex;
  std::vector<vec3> _direction_0;
  for (auto _one_of_controlpoints : control_points_m_)
    _direction_0.push_back(BezierCurve(_one_of_controlpoints).evaluate(u).position);
  _vertex = BezierCurve(_direction_0).evaluate(v);
  // meanwhile _vertex.normal is tangent along v
  Vertex _vertex_0;
  std::vector<vec3> _direction_1;
  for (auto _one_of_controlpoints : control_points_n_)
    _direction_1.push_back(BezierCurve(_one_of_controlpoints).evaluate(v).position);
  _vertex_0 = BezierCurve(_direction_1).evaluate(u);
  // meanwhile _vertex_0.normal is tangent along u
  // assert(_vertex.position == _vertex_0.position)
  _vertex.normal = glm::normalize(glm::cross(_vertex.normal, _vertex_0.normal));
  return _vertex;
}

Vertex BezierSurface::evaluate(
    float u, float v)
{
  Vertex _vertex;
  std::vector<vec3> _direction_0;
  for (auto _one_of_controlpoints : control_points_m_)
    _direction_0.push_back(BezierCurve(_one_of_controlpoints).evaluate(u).position);
  _vertex = BezierCurve(_direction_0).evaluate(v);
  // meanwhile _vertex.normal is tangent along v
  Vertex _vertex_0;
  std::vector<vec3> _direction_1;
  for (auto _one_of_controlpoints : control_points_n_)
    _direction_1.push_back(BezierCurve(_one_of_controlpoints).evaluate(v).position);
  _vertex_0 = BezierCurve(_direction_1).evaluate(u);
  // meanwhile _vertex_0.normal is tangent along u
  // assert(_vertex.position == _vertex_0.position)
  _vertex.normal = glm::normalize(glm::cross(_vertex.normal, _vertex_0.normal));
  return _vertex;
}

/**
 * TODO: generate an Object of the current Bezier surface
 */
Object BezierSurface::generateObject()
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
std::vector<BezierSurface> read(const std::string &path)
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

  std::vector<BezierSurface> _surfaces;
  for (int k = 0; k < b; ++k)
  {
    BezierSurface _beziersurface(m, n);
    for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j)
        _beziersurface.setControlPoint(i, j, point_pos[ctr_points[k][i * n + j]]);
    _surfaces.push_back(_beziersurface);
  }
  return _surfaces;
}