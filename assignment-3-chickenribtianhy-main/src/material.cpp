#include "material.h"

#include <utility>
#include <iostream>

ConstColorMat::ConstColorMat() : diffuse_color({1, 1, 1}),
                                 specular_color({1, 1, 1}),
                                 ambient_color(1, 1, 1),
                                 shininess(16.f)
{
  type = Material::Type::COLOR;
}

ConstColorMat::ConstColorMat(const Vec3f &color, float sh)
    : diffuse_color(color), specular_color(color), ambient_color(color), shininess(sh) {}

ConstColorMat::ConstColorMat(Vec3f diff, Vec3f spec, Vec3f ambient, float sh)
    : diffuse_color(std::move(diff)),
      specular_color(std::move(spec)),
      ambient_color(std::move(ambient)),
      shininess(sh) {}

InteractionPhongLightingModel ConstColorMat::evaluate(Interaction &interaction) const
{
  InteractionPhongLightingModel m;
  // TODO: Your code here.
  m.ambient = ambient_color;
  m.diffusion = diffuse_color;
  m.shininess = shininess;
  m.specular = specular_color;
  // printf("faq\n");
  return m;
}

TextureMat::TextureMat()
{
  tex_diff = new Texture();
  tex_disp = new Texture();
  tex_norm = new Texture();
  type = Material::Type::TEXTURE;
}

void TextureMat::set_diff_map(const std::string &diff_path)
{
  tex_diff->loadTextureFromFile(diff_path);
  tex_diff->type = Texture::DIFF;
}

void TextureMat::set_disp_map(const std::string &disp_path)
{
  tex_disp->loadTextureFromFile(disp_path);
  tex_disp->type = Texture::DISP;
}

void TextureMat::set_norm_map(const std::string &norm_path)
{
  tex_norm->loadTextureFromFile(norm_path);
  tex_norm->type = Texture::NORM;
}

InteractionPhongLightingModel TextureMat::evaluate(Interaction &interaction) const
{

  InteractionPhongLightingModel m;
  Vec3f res;
  res = tex_diff->interpolation(interaction.uv.x(), interaction.uv.y());
  // std::cout<<"faq"<<std::endl;
  m.ambient = res;
  m.diffusion = res;
  m.specular = res;
  m.shininess = 16.0f;
  return m;
}

GroundMat::GroundMat()
{
  tex_diff = new Texture();
  tex_disp = new Texture();
  tex_norm = new Texture();
  type = Material::Type::GRID;
}
void GroundMat::set_diff_map(const std::string &diff_path)
{
  tex_diff->loadTextureFromFile(diff_path);
  tex_diff->type = Texture::DIFF;
  // tex_diff->generate_MipMap();
}

InteractionPhongLightingModel GroundMat::evaluate(Interaction &interaction) const
{
  if (!tex_diff->is_mipmaped)
  {
    InteractionPhongLightingModel m;
    Vec3f res;

    float u = interaction.uv.x();
    float v = interaction.uv.y();
    // std::cout<<"faq"<<std::endl;
    // Vec2f pos;
    // pos = Vec2f(u, v).cwiseProduct(Vec2f((float)(tex_diff->w), (float)(tex_diff->h)));

    // int x = (int)pos.x();
    // int y = (int)pos.y();

    // int p00 = x * tex_diff->h + y;
    // res = tex_diff->get_data(p00);
    res = tex_diff->get_data_uv(u, v) / 255.0f;
    // std::cout << "res = " << res << std::endl;

    m.ambient = res;
    m.diffusion = res;
    m.specular = res;
    m.shininess = 16.0f;
    return m;
  }
  InteractionPhongLightingModel m;

  float duv, dxy;
  float u = interaction.uv.x();
  float v = interaction.uv.y();
  // std::cout << "faq\n";
  // float u2 = interaction.another_in->uv.x();
  // float v2 = interaction.another_in->uv.y();

  // dudy = u2 - u;
  // dvdy = v2 - v;
  // float qr = dudy * dudy + dvdy * dvdy;
  // // qr = sqrt(qr);
  // printf("%f, %f, %f, %f\n", u, v, u2, v2);
  // // std::cout << " = " << qr << std::endl;
  // std::cout << "qr = " << qr << std::endl;
  float op = interaction.dist;
  float oa = interaction.dist_f;
  float cos = interaction.normal.dot(interaction.cam_dir);

  float qr = op / oa / 128;
  qr /= cos;
  // #define MAXLEN 8.0f

  //   duv = 2 * interaction.dist / interaction.normal.dot(interaction.cam_dir);
  // dxy = MAXLEN / tex_diff->w;
  // float _theta = 60 / 2 * PI / 180;
  // float tg = tanf(_theta) / 400;

  // duv *= tg;
  // float qr = duv / dxy;
  // std::cout << "qr = " << qr << std::endl;
  float L = log2f(qr);
  int L_int = floor(L);
  L_int = std::max(0, std::min(L_int, tex_diff->_layer_num - 1));
  float delta = L - (float)L_int;
  int L_int_2 = std::min(L_int + 1, tex_diff->_layer_num - 1);

  // float dist_uv = -2 * interaction.dist * interaction.cam_factor / interaction.normal.dot(interaction.in_direction);
  // float dist_tex = (float)interaction.tex_width / tex_diff->w;

  // std::cout << "L = " << L_int << " " << L_int_2 << std::endl;

  // std::cout << "pox = " << interaction.pos << std::endl;
  // std::cout << ", op = " << op;
  // std::cout << ", oa = " << oa << std::endl;
  // L_int = 0;
  Vec3f p1 = tex_diff->get_data_uv(u, v, L_int) / 255.0f;
  Vec3f p2 = tex_diff->get_data_uv(u, v, L_int_2) / 255.0f;

  Vec3f pp = (1 - delta) * p1 + delta * p2;
  // std::cout << "p1 = " << p1 << std::endl;
  // p1 = Vec3f(112, 112, 112);
  // if (L_int == 0)
  //   m.ambient = p1;
  // else if (L_int == 1 && delta < 1)
  //   m.ambient = p1;
  // else
  m.ambient = pp;
  // m.ambient = p2;

  // m.diffusion = pp;
  // m.specular = pp;
  m.shininess = 16.0f;
  return m;
}
