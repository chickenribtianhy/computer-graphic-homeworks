#ifndef CS171_HW3_INCLUDE_INTERACTION_H_
#define CS171_HW3_INCLUDE_INTERACTION_H_

#include "core.h"

struct InteractionPhongLightingModel
{
  Vec3f diffusion;
  Vec3f specular;
  Vec3f ambient;
  float shininess{};
};

struct Interaction
{
  enum Type
  {
    LIGHT,
    GEOMETRY,
    NONE,
    GROUND
  };
  Vec3f pos{};
  float dist{RAY_DEFAULT_MAX};
  Vec3f normal{};
  Vec2f uv{};
  InteractionPhongLightingModel model;
  Type type{Type::NONE};
  float dist_f{};
  // struct Interaction *another_in{};
  Vec3f cam_dir{};
};

#endif // CS171_HW3_INCLUDE_INTERACTION_H_
