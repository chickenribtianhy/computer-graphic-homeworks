#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "texture.h"
#include <iostream>
void Texture::loadTextureFromFile(const std::string &path)
{
  int w, h, ch;
  stbi_uc *img = stbi_load(path.c_str(), &w, &h, &ch, 0);
  tex_data.assign(img, img + w * h * ch);
  stbi_image_free(img);

  this->w = w;
  this->h = h;
  this->ch = ch;
  // std::cout << "load data success" << std::endl;
  // std::cout << tex_data.size() << std::endl;
}

Texture::Texture()
{
  type = Texture::NONE;
  is_mipmaped = false;
}

Vec3f Texture::get_data(int index)
{

  return Vec3f((float)tex_data[index * 3], (float)tex_data[index * 3 + 1], (float)tex_data[index * 3 + 2]);
}

Vec3f Texture::get_data(int index, int layer)
{
  Vec3f res((float)layers[layer][index * 3], (float)layers[layer][index * 3 + 1], (float)layers[layer][index * 3 + 2]);

  // std::cout << "res = " << res << std::endl;
  return res;
}

Vec3f Texture::get_data_uv(float u, float v, int layer)
{
  // printf("faq %d\n", layer);
  int _s = (int)powf(2, layer);
  int new_w = w / _s;
  int new_h = h / _s;
  Vec2f pos;
  pos = Vec2f(u, v).cwiseProduct(Vec2f((float)(new_w), (float)(new_h)));

  int x = (int)pos.x();
  int y = (int)pos.y();

  int p00 = x * new_h + y;

  // printf("p00 = %d\n", p00);
  return get_data(p00, layer);
}

Vec3f Texture::get_data_uv(float u, float v)
{
  Vec2f pos;
  pos = Vec2f(u, v).cwiseProduct(Vec2f((float)(w), (float)(h)));

  int x = (int)pos.x();
  int y = (int)pos.y();

  int p00 = x * h + y;
  return get_data(p00);
}

Vec3f Texture::interpolation(float u, float v)
{

  Vec2f pos;
  pos = Vec2f(u, v).cwiseProduct(Vec2f((float)w, (float)h));
  int x = (int)pos.x();
  int y = (int)pos.y();
  // int u0 = std::min(x + 1, w - 1);
  int u0 = x + 1;
  if (u0 > w - 1)
  {
    u0 = w - 1;
  }
  // int v0 = std::min(y + 1, h - 1);
  int v0 = y + 1;
  if (v0 > h - 1)
  {
    v0 = h - 1;
  }

  int p00 = x * h + y;
  int p01 = x * h + v0;
  int p10 = u0 * h + y;
  int p11 = u0 * h + v0;
  // printf("faq %d\n", p00);
  Vec3f point_00 = get_data(p00);
  Vec3f point_01 = get_data(p01);
  Vec3f point_10 = get_data(p10);
  Vec3f point_11 = get_data(p11);

  float k1;
  float k2;
  if (u0 == x) // margin
  {
    k1 = 0;
  }
  if (v0 == y)
  {
    k2 = 0;
  }

  if (u0 != x && v0 != y)
  {
    k1 = (u0 - pos.x()) / ((float)u0 - x);
    k2 = (v0 - pos.y()) / ((float)v0 - y);
  }
  Vec3f down = (1 - k1) * point_00 + k1 * point_10;
  Vec3f up = (1 - k1) * point_01 + k1 * point_11;

  return ((1 - k2) * down + k2 * up) / 255.f;
}

void Texture::generate_MipMap()
{
  // std::vector<std::vector<uint8_t>> layers;
  int layer_num = 0;
  int curr_w = w;
  int curr_h = h;
  is_mipmaped = true;
  layers.push_back(tex_data);

  while (curr_w > 1 && curr_h > 1)
  {
    curr_w /= 2;
    curr_h /= 2;

    std::vector<uint8_t> layer;
    // traverse
    for (int i = 0; i < curr_h; ++i)
    {
      for (int j = 0; j < curr_w; ++j)
      {
        // ch = 3
        for (int k = 0; k < ch; ++k)
        {
          float _x = 0;
          _x += (float)(layers[layer_num][((2 * i) * curr_w * 2 + 2 * j) * 3 + k]);
          _x += (float)(layers[layer_num][((2 * i) * curr_w * 2 + 2 * j + 1) * 3 + k]);
          _x += (float)(layers[layer_num][((2 * i + 1) * curr_w * 2 + 2 * j) * 3 + k]);
          _x += (float)(layers[layer_num][((2 * i + 1) * curr_w * 2 + 2 * j + 1) * 3 + k]);
          _x /= 4;
          // std::cout << "layer = " << layer_num + 1 << "_x = " << _x << std::endl;
          layer.push_back((uint8_t)_x);
        }
      }
    }
    ++layer_num;
    layers.push_back(layer);
  }
  _layer_num = layer_num + 1;
  return;
}