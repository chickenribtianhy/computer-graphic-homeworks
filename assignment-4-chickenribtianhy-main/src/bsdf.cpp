#include "bsdf.h"
#include "utils.h"

#include <utility>

Vec3f global_up(0.f, 0.f, 1.f);

IdealDiffusion::IdealDiffusion(const Vec3f &color) : color(color) {}

Vec3f IdealDiffusion::evaluate(Interaction &interaction) const
{
  // TODO: your implementation here.
  Vec3f ret;
  ret = color * INV_PI;
  return ret;
}

float IdealDiffusion::pdf(Interaction &interaction) const
{
  // TODO: your implementation here
  float obj_cos = (interaction.wo).dot(interaction.normal);
  return obj_cos * INV_PI;
}

float IdealDiffusion::sample(Interaction &interaction, Sampler &sampler) const
{
  // TODO: your implementation here
  Vec2f cauchy = sampler.get2D();
  float theta_local = asin(sqrt(cauchy.x()));
  float phi_local = 2 * PI * cauchy.y();
  Vec3f dir(sin(theta_local) * cos(phi_local), sin(theta_local) * sin(phi_local), cos(theta_local)); //xyz
  Mat3f local2global = Eigen::Quaternionf::FromTwoVectors(global_up, interaction.normal).toRotationMatrix();
  interaction.wo = (local2global * dir).normalized();
  return pdf(interaction);
}
/// return whether the bsdf is perfect transparent or perfect reflection
bool IdealDiffusion::isDelta() const
{
  return false;
}

IdealSpecular::IdealSpecular(const Vec3f &color) : color(color) {}

Vec3f IdealSpecular::evaluate(Interaction &interaction) const
{
  return color;
}

float IdealSpecular::pdf(Interaction &interaction) const
{
  return 1.0f;
}

float IdealSpecular::sample(Interaction &interaction, Sampler &sampler) const
{
  float cos_theta = (-interaction.wi).dot(interaction.normal);
  interaction.wo = (2 * cos_theta * interaction.normal + interaction.wi).normalized();
  return pdf(interaction);
}

bool IdealSpecular::isDelta() const
{
  return true;
}

float FrDielectric(float cosThetaI, float etaI, float etaT)
{
  cosThetaI = std::clamp(cosThetaI, -1.f, 1.f);
  if (cosThetaI < 0.f)
  {
    std::swap(etaI, etaT);
    cosThetaI = std::abs(cosThetaI);
  }
  float sinThetaI = std::sqrt(std::max(0.f, 1 - cosThetaI * cosThetaI));
  float sinThetaT = etaI / etaT * sinThetaI;
  float cosThetaT = std::sqrt(std::max(0.f, 1 - sinThetaT * sinThetaT));
  float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
  float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
  return (Rparl * Rparl + Rperp * Rperp) / 2;
}

bool Refract(const Vec3f &wi, const Vec3f &n, float eta, Vec3f *wo)
{
  float cosThetaI = std::abs(n.dot(wi));
  float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
  float sin2ThetaT = eta * eta * sin2ThetaI;
  if (sin2ThetaT >= 1.f) //reflect
  {
    return false;
  }
  float cosThetaT = std::sqrt(1 - sin2ThetaT);
  *wo = ((-cosThetaT + eta * cosThetaI) * n + eta * (-wi)).normalized(); //
  return true;
}

Translucent::Translucent(const Vec3f &color) : color(color) {}
Translucent::Translucent(const Vec3f &color, float eta_in, float eta_out) : color(color), eta_in(eta_in), eta_out(eta_out) {}

Vec3f Translucent::evaluate(Interaction &interaction) const
{
  float cos_theta = (-interaction.wi).dot(interaction.normal);
  Vec3f reflect = (2 * cos_theta * interaction.normal + interaction.wi).normalized();
  if (interaction.wo.dot(reflect) >= 1 - 1e-5)
  {
    return color;
  } //reflected
  else
  {
    float etaI;
    float etaT;
    if (cos_theta > 0)
    {
      etaI = eta_in;
      etaT = eta_out;
    }
    else
    {
      etaI = eta_out;
      etaT = eta_in;
    }
    float Fr = FrDielectric(cos_theta, etaI, etaT);
    float coeff = etaI * etaI / (etaT * etaT); //
    return coeff * (1 - Fr) * color;
  }
}
float Translucent::pdf(Interaction &interaction) const
{
  return 1.0f;
}
float Translucent::sample(Interaction &interaction, Sampler &sampler) const
{
  float cos_theta = (-interaction.wi).dot(interaction.normal);
  float etaI;
  float etaT;
  if (cos_theta > 0)
  {
    etaI = eta_in;
    etaT = eta_out;
  }
  else
  {
    etaI = eta_out;
    etaT = eta_in;
  }
  bool refracted = Refract(-interaction.wi, interaction.normal, etaI / etaT, &interaction.wo);
  if (!refracted)
  {
    interaction.wo = (2 * cos_theta * interaction.normal + interaction.wi).normalized();
  } //reflected
  return pdf(interaction);
}
bool Translucent::isDelta() const
{
  return true;
}
