#ifndef BSDF_H_
#define BSDF_H_

#include "interaction.h"

class BSDF
{
public:
  BSDF() = default;
  virtual ~BSDF() = default;
  [[nodiscard]] virtual Vec3f evaluate(Interaction &interaction) const = 0;
  virtual float pdf(Interaction &interaction) const = 0;
  virtual float sample(Interaction &interaction, Sampler &sampler) const = 0;
  [[nodiscard]] virtual bool isDelta() const = 0;
};

class IdealDiffusion : public BSDF
{
public:
  explicit IdealDiffusion(const Vec3f &color);
  [[nodiscard]] Vec3f evaluate(Interaction &interaction) const override;
  float pdf(Interaction &interaction) const override;
  float sample(Interaction &interaction, Sampler &sampler) const override;
  [[nodiscard]] bool isDelta() const override;

private:
  Vec3f color;
};

// You can add your own bsdf here

class IdealSpecular : public BSDF
{
public:
  explicit IdealSpecular(const Vec3f &color);
  [[nodiscard]] Vec3f evaluate(Interaction &interaction) const override;
  float pdf(Interaction &interaction) const override;
  float sample(Interaction &interaction, Sampler &sampler) const override;
  [[nodiscard]] bool isDelta() const override;

private:
  Vec3f color;
};

class Translucent : public BSDF
{
public:
  explicit Translucent(const Vec3f &color);
  explicit Translucent(const Vec3f &color, float eta_in, float eta_out);
  [[nodiscard]] Vec3f evaluate(Interaction &interaction) const override;
  float pdf(Interaction &interaction) const override;
  float sample(Interaction &interaction, Sampler &sampler) const override;
  [[nodiscard]] bool isDelta() const override;

private:
  Vec3f color;
  float eta_in = 1.0f;
  float eta_out = 1.5f;
};

#endif //BSDF_H_
