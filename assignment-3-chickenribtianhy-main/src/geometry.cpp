#include "geometry.h"

#include <utility>
#include <iostream>

Triangle::Triangle(Vec3f v0, Vec3f v1, Vec3f v2)
    : v0(std::move(v0)), v1(std::move(v1)), v2(std::move(v2))
{
  normal = (v1 - v0).cross(v2 - v1).normalized();
}

bool Triangle::intersect(Ray &ray, Interaction &interaction) const
{
  // TODO: Your code here.
  Vec3f e1 = v1 - v0;
  Vec3f e2 = v2 - v0;
  Vec3f s0 = ray.origin - v0;
  Vec3f s1 = ray.direction.cross(e2);
  Vec3f s2 = s0.cross(e1);

  float t = s2.dot(e2) / s1.dot(e1);
  float b1 = s1.dot(s0) / s1.dot(e1);
  float b2 = s2.dot(ray.direction) / s1.dot(e1);

  if (t > 0 && b1 >= 0 && b2 >= 0 && 1 - b1 - b2 >= 0)
  {
    // in the triangle
    if (t < interaction.dist)
    {
      interaction.type = Interaction::Type::GEOMETRY;
      interaction.pos = ray.origin + t * ray.direction;
      interaction.dist = t;
      interaction.normal = normal;
      interaction.uv = Vec2f(b1, b2);
      interaction.model = material->evaluate(interaction);
      return true;
    }
  }
  return false;
}

Rectangle::Rectangle(Vec3f position, Vec2f dimension, Vec3f normal, Vec3f tangent)
    : Geometry(),
      position(std::move(position)),
      size(std::move(dimension)),
      normal(std::move(normal)),
      tangent(std::move(tangent)) {}

bool Rectangle::intersect(Ray &ray, Interaction &interaction) const
{
  // TODO: Your code here
  Vec3f n = normal.normalized();
  float t = (position - ray.origin).dot(n) / (ray.direction.dot(n));
  Vec3f P = ray.origin + t * ray.direction; // o+td
  Vec3f P0_P = P - position;

  Vec3f tg = this->tangent.normalized();
  float px = P0_P.dot(tg);

  Vec3f ctg = n.cross(tg).normalized();
  float py = P0_P.dot(ctg);

  ConstColorMat color_mat;

  // float pixel_x, pixel_y;

  Vec3f text_tg = tangent.normalized();
  Vec3f text_ctg = n.cross(text_tg).normalized();
  // pixel_x = P0_P.dot(text_tg);
  // pixel_y = P0_P.dot(text_ctg);

  // printf("abs(px) = %f, size.x = %f\n", abs(py), size.y());
  // printf("t = %f\n", t);
  // printf("ray_origin = %f\n", t);
  // std::cout << "ray_origin = " << n << std::endl;
  float size_x = size.x() / 2;
  float size_y = size.y() / 2;
  // if (px <= this->size.x() && std::abs(py) <= this->size.y() && t > 0.0f)
  if (px <= size_x && px >= -size_x && py <= size_y && py >= -size_y && t > 0.0f)
  {
    // printf("faq rect\n");
    if (t < interaction.dist)
    {
      interaction.type = Interaction::Type::GEOMETRY;
      interaction.pos = P;
      interaction.dist = t;
      interaction.uv = Vec2f(px / this->size.x() + 0.5f, py / this->size.y() + 0.5f);

      // interaction.normal = n;
      // printf("faqrec: ");
      if (material != nullptr)
      {
        if (material->type == Material::Type::TEXTURE && material->tex_norm != nullptr)
        // if (this->material->tex_norm != nullptr)
        {
          // texture normal
          Vec2f pos;
          Vec3f local_normal, world_normal;
          pos = Vec2f(interaction.uv.x(), interaction.uv.y()).cwiseProduct(Vec2f((float)material->tex_norm->w, (float)material->tex_norm->h));
          local_normal = material->tex_norm->get_data((int)pos.x() * material->tex_norm->h + (int)pos.y());

          world_normal = local_normal.x() * text_tg + local_normal.y() * text_ctg + local_normal.z() * n;
          interaction.normal = world_normal.normalized();
          // interaction.normal = n;
          // std::cout << "normal = " << interaction.normal << std::endl;
          // interaction.normal = n;
          // Vec3f local_disp, world_disp;

          // interaction.dist = t;
        }
        else
        {
          interaction.normal = n; // normal of rect
        }
        interaction.model = this->material->evaluate(interaction);
      }
      else
      {
        interaction.normal = n;
        interaction.model = color_mat.evaluate(interaction);
      }
      return true;
    }
  }
  return false;
}
Vec2f Rectangle::getSize() const
{
  return size;
}
Vec3f Rectangle::getNormal() const
{
  return normal;
}
Vec3f Rectangle::getTangent() const
{
  return tangent;
}

Ellipsoid::Ellipsoid(const Vec3f &p, const Vec3f &a, const Vec3f &b, const Vec3f &c)
    : p(p), a(a), b(b), c(c) {}

bool Ellipsoid::intersect(Ray &ray, Interaction &interaction) const
{
  // TODO: Your code here.
  Eigen::Matrix4f S, R, T;
  T << 1.0f, 0.0f, 0.0f, p(0),
      0.0f, 1.0f, 0.0f, p(1),
      0.0f, 0.0f, 1.0f, p(2),
      0.0f, 0.0f, 0.0f, 1.0f;
  S << a.norm(), 0.0f, 0.0f, 0.0f,
      0.0f, b.norm(), 0.0f, 0.0f,
      0.0f, 0.0f, c.norm(), 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f;

  Vec3f a_norm = a.normalized(), b_norm = b.normalized(), c_norm = c.normalized();
  R << a_norm(0), b_norm(0), c_norm(0), 0.0f,
      a_norm(1), b_norm(1), c_norm(1), 0.0f,
      a_norm(2), b_norm(2), c_norm(2), 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f;

  Eigen::Matrix4f M = T * R * S;
  Eigen::Matrix4f M_inv = M.inverse();

  Vec4f o_4d = M_inv * Vec4f(ray.origin(0), ray.origin(1), ray.origin(2), 1.0f);
  Vec4f d_4d = M_inv * Vec4f(ray.direction(0), ray.direction(1), ray.direction(2), 0.0f);

  Vec3f o = Vec3f(o_4d(0), o_4d(1), o_4d(2));
  Vec3f d = Vec3f(d_4d(0), d_4d(1), d_4d(2));
  float d_norm = d.norm();
  d /= d_norm;

  Vec3f L = Vec3f(0.0f, 0.0f, 0.0f) - o;
  float t_ca = L.dot(d);
  if (t_ca > 0.0f)
  {
    float d2 = L.dot(L) - t_ca * t_ca;
    if (d2 <= 1.0f)
    {
      float t_nc = std::sqrt(1.0f - d2);
      float t = t_ca - t_nc;
      if (t / d_norm < interaction.dist)
      {
        interaction.type = Interaction::Type::GEOMETRY;
        interaction.pos = ray.origin + t / d_norm * ray.direction;
        interaction.dist = t / d_norm;
        Vec3f n_sphere = o + t * d;

        Vec4f n_4d = M.inverse().transpose() * Vec4f(n_sphere(0), n_sphere(1), n_sphere(2), 0.0f);
        interaction.normal = Vec3f(n_4d(0), n_4d(1), n_4d(2));
        interaction.model = material->evaluate(interaction);
        return true;
      }
    }
  }
  return false;
}

Ground::Ground(float _h)
{
  h = _h;
}

#define MAXLEN 8.0f
Vec2f get_uv(Vec3f pp)
{
  float _m1 = fmod(pp.x(), MAXLEN);
  float _m2 = fmod(pp.z(), MAXLEN);
  if (_m1 < 0)
    _m1 += MAXLEN;
  if (_m2 < 0)
    _m2 += MAXLEN;
  _m1 = _m1 / MAXLEN;
  _m2 = _m2 / MAXLEN;
  return Vec2f(_m1, _m2);
}

bool Ground::intersect(Ray &ray, Interaction &interaction) const
{
  // TODO: Your code here
  float t = (h - ray.origin.y()) / ray.direction.y();
  // std::cout << "ray_dir = " << ray.direction << std::endl;
  // std::cout << "h = " << t << std::endl;
  // std::cout << "ray_origin = " << ray.origin << std::endl;
  // std::cout << "t = " << t << std::endl;
  if (ray.t_min <= t && t <= ray.t_max)
  {
    interaction.type = Interaction::Type::GROUND;
    interaction.pos = ray.origin + t * ray.direction;
    interaction.dist = t;
    interaction.uv = get_uv(interaction.pos);
    interaction.normal = Vec3f(0, 1, 0);
    interaction.cam_dir = ray.direction;
    if (material)
    {
      interaction.model = material->evaluate(interaction);
    }
    return true;
  }
  return false;
}