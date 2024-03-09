#ifndef CS171_HW3_INCLUDE_TEXTURE_H_
#define CS171_HW3_INCLUDE_TEXTURE_H_

#include <vector>
#include <string>

#include "core.h"

class Texture
{
public:
  Texture();
  enum Type
  {
    DIFF,
    NORM,
    DISP,
    NONE
  };
  Type type;
  void loadTextureFromFile(const std::string &path);

  Vec3f get_data(int index);
  Vec3f get_data(int index, int layer);
  Vec3f get_data_uv(float u, float v);
  Vec3f get_data_uv(float u, float v, int layer);
  Vec3f interpolation(float u, float v);
  int w, h, ch;

  void generate_MipMap();
  bool is_mipmaped;
  int _layer_num;

private:
  std::vector<uint8_t> tex_data;
  std::vector<std::vector<uint8_t>> layers;
};

#endif // CS171_HW3_INCLUDE_TEXTURE_H_
