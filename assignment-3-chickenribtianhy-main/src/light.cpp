#include "light.h"

constexpr uint32_t SAMPLE_NUM = 16;

Light::Light(const Vec3f &pos, const Vec3f &color) : position(pos), color(color)
{
}
Vec3f Light::getColor() const
{
  return color;
}

SquareAreaLight::SquareAreaLight(const Vec3f &pos, const Vec3f &color, const Vec2f &dimension,
                                 const Vec3f &normal,
                                 const Vec3f &tangent) : Light(pos, color),
                                                         rectangle(pos, dimension, normal, tangent) {}

#define SAMPLE_X 15
#define SAMPLE_Y 9
std::vector<LightSample> SquareAreaLight::samples() const
{
  std::vector<LightSample> samples;
  // TODO: Your code here.
  Vec2f rec_s = this->rectangle.getSize();
  Vec3f tg = this->rectangle.getTangent().normalized();
  Vec3f ctg = this->rectangle.getNormal().cross(tg).normalized();
  for (int x = 0; x < SAMPLE_X; ++x)
  {
    for (int y = 0; y < SAMPLE_Y; ++y)
    {
      float pos_x = (2 * ((float)x + 0.5f) / SAMPLE_X - 1) * rec_s.x() / 2;
      float pos_y = (2 * ((float)y + 0.5f) / SAMPLE_Y - 1) * rec_s.y() / 2;
      LightSample s;
      s.color = color;
      s.position = position + pos_x * tg + pos_y * ctg;
      samples.push_back(s);
    }
  }
  return samples;
}
bool SquareAreaLight::intersect(Ray &ray, Interaction &interaction) const
{
  // TODO: Your code here.

  if (rectangle.intersect(ray, interaction))
  {
    interaction.type = Interaction::Type::LIGHT;
    return true;
  }
  return false;
}
