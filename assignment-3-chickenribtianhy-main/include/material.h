#ifndef CS171_HW3_INCLUDE_MATERIAL_H_
#define CS171_HW3_INCLUDE_MATERIAL_H_

#include "interaction.h"
#include "texture.h"
class Material
{
public:
  Material()
  {
    tex_norm = nullptr;
    tex_diff = nullptr;
    tex_disp = nullptr;
  }
  virtual ~Material() = default;
  [[nodiscard]] virtual InteractionPhongLightingModel evaluate(Interaction &interaction) const = 0;
  enum Type
  {
    COLOR,
    TEXTURE,
    GRID
  };
  Type type;

  Texture *tex_disp, *tex_diff, *tex_norm;
};

class ConstColorMat : public Material
{
public:
  ConstColorMat();
  explicit ConstColorMat(const Vec3f &color, float sh = 16.f);
  ConstColorMat(Vec3f diff, Vec3f spec, Vec3f ambient, float sh = 16.f);
  [[nodiscard]] InteractionPhongLightingModel evaluate(Interaction &interaction) const override;

private:
  Vec3f diffuse_color;
  Vec3f specular_color;
  Vec3f ambient_color;
  float shininess;
};

class TextureMat : public Material
{
  // TODO: Bonus - texture material
public:
  TextureMat();
  void set_disp_map(const std::string &disp_path);
  void set_diff_map(const std::string &diff_path);
  void set_norm_map(const std::string &norm_path);
  [[nodiscard]] InteractionPhongLightingModel evaluate(Interaction &interaction) const override;
};

class GroundMat : public Material
{
public:
  GroundMat();
  void set_diff_map(const std::string &diff_path);
  [[nodiscard]] InteractionPhongLightingModel evaluate(Interaction &interaction) const override;
};

#endif // CS171_HW3_INCLUDE_MATERIAL_H_
