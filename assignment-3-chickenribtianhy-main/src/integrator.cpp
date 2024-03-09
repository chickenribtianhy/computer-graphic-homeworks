#include "integrator.h"
#include <omp.h>
#include <iostream>
#include <utility>
// #include <math.h>
#define SAMPLE_RATE 10

#define SAMPLE_RATE_X 1
#define SAMPLE_RATE_Y 1

Vec3f dot_vector_(Vec3f x, Vec3f y)
{
  Vec3f r = Vec3f(x(0) * y(0), x(1) * y(1), x(2) * y(2));
  return r;
}

PhongLightingIntegrator::PhongLightingIntegrator(std::shared_ptr<Camera> cam,
                                                 std::shared_ptr<Scene> scene)
    : camera(std::move(cam)), scene(std::move(scene))
{
}

void PhongLightingIntegrator::render() const
{
  Vec2i resolution = camera->getImage()->getResolution();
  int cnt = 0;
  // if ()
  std::vector<LightSample> light_samples = this->scene->getLight()->samples();
  // printf("faq\n");
  std::vector<Vec2f> points;
  // if (scene->getLight()->getColor() == Vec3f(0, 0, 0))
  // {
  //   points.push_back(Vec2f(0, 0));
  // }
  // else
  {
    Eigen::Matrix2f T;
    // T << sqrt(2) / 5, -1 / 5,
    //     1 / 5, sqrt(2) / 5;
    float sin = 1 / sqrt(5);
    float cos = 2 / sqrt(5);
    float offset = (1.0f / SAMPLE_RATE_X) / 2;
    float offset2 = (1.0f / SAMPLE_RATE_Y) / 2;
    for (int i = 0; i < SAMPLE_RATE_X; ++i)
    {
      for (int j = 0; j < SAMPLE_RATE_Y; ++j)
      {
        float x = i * 1.0f / SAMPLE_RATE_X - 0.5f + offset;
        float y = j * 1.0f / SAMPLE_RATE_Y - 0.5f + offset2;
        // pp = T * pp;
        float a = cos * x - sin * y;
        float b = sin * x + cos * y;
        Vec2f pp(a, b);
        // Vec2f pp(x, y);
        // std::cout << pp.x() << " " << pp.y() << std::endl;
        points.push_back(pp);
      }
    }
  }
  // std::cout << "ffafafafafafaf" << std::endl;
  // for (int i = 0; i < SAMPLE_RATE_X * SAMPLE_RATE_Y; ++i)
  // {
  //   std::cout << points[i].x() << " " << points[i].y() << std::endl;
  // }

  // srand(0);
  // #pragma omp parallel for schedule(guided, 2), shared(cnt)
  for (int dx = 0; dx < resolution.x(); dx++)
  {
    // #pragma omp atomic
    ++cnt;
    printf("faq %d\n", dx);
    printf("\r%.02f%%", cnt * 100.0 / resolution.x());
    // printf("faq%.02f\n", cnt * 100.0 / resolution.x());
    for (int dy = 0; dy < resolution.y(); dy++)
    {
      // std::cout << "pixel : " << std::endl;
      Vec3f L(0, 0, 0);
      // TODO: Your code here.
      // for (int i = 0; i < SAMPLE_RATE; ++i)

      for (auto pp : points)
      {
        float pos_x = dx + pp.x();
        float pos_y = dy + pp.y();
        // float pos_x = dx + (float)rand() / RAND_MAX - 0.5f;
        // float pos_y = dy + (float)rand() / RAND_MAX - 0.5f;

        // auto img_resolution = image->getResolution();
        // auto img_aspect = image->getAspectRatio();

        Ray ray = camera->generateRay(pos_x, pos_y);
        Interaction interaction;
        float px = (pos_x / resolution.x() * 2 - 1) * camera->scale;
        float py = (pos_y / resolution.y() * 2 - 1) * camera->scale;
        float dist_op_sq = px * px + py * py;
        // std::cout << "scale = " << camera->scale << std::endl;
        // std::cout << "dx = " << dx << std::endl;
        // std::cout << "px = " << px << std::endl;
        // std::cout << "dist_op_sq = " << dist_op_sq << std::endl;
        float dist_focal = dist_op_sq + camera->get_focal_len() * camera->get_focal_len();
        dist_focal = sqrt(dist_focal);
        // std::cout << "dist_focal = " << dist_focal << std::endl;
        interaction.dist_f = dist_focal;

        // std::cout << "faq1 " << std::endl;
        // interaction.dist = MAX_DIST;
        bool hit = scene->intersect(ray, interaction);
        Vec3f _ray_(0.0f, 0.0f, 0.0f);
        // printf("hit = %d\n", hit);
        if (hit)
        {
          // std::printf("dist = %f\n", interaction.dist);
          if (interaction.type == Interaction::Type::LIGHT)
          {
            _ray_ = scene->getLight()->getColor();
          }
          else if (interaction.type == Interaction::Type::GROUND)
          {
            // Ray ray_2 = camera->generateRay(pos_x, pos_y++);
            // Interaction interaction_2;
            // std::cout << "faq2 " << std::endl;
            // scene->intersect(ray_2, interaction_2);
            // interaction.another_in = &interaction_2;
            // std::cout << "faq3 " << std::endl;

            // hit = scene->intersect(ray, interaction);
            _ray_ = dot_vector_(scene->getAmbient(), interaction.model.ambient);
          }
          else
          {
            _ray_ = Vec3f(0.0f, 0.0f, 0.0f);
            for (auto light : light_samples)
            {
              Ray ref(interaction.pos + 1e-5 * interaction.normal, (light.position - interaction.pos).normalized());
              // Ray ref(interaction.pos + 1e-3 * interaction.normal, (light.position - interaction.pos).normalized());
              Interaction interaction_2;
              // printf("faq\n");
              // if (!scene->isShadowed(ref))
              if (scene->intersect(ref, interaction_2) && interaction_2.type == Interaction::Type::LIGHT)
              {
                _ray_ += radiance(ray, interaction, light);
              }
              else
              {
                _ray_ += dot_vector_(scene->getAmbient(), interaction.model.ambient);
              }
              // std::cout << "_ray_ = " << _ray_ << std::endl;
              // break;
            }
            _ray_ /= light_samples.size();
          }
          L += _ray_;
        }
      }
      // L /= SAMPLE_RATE;
      // L /= (SAMPLE_RATE_X * SAMPLE_RATE_Y);
      L /= points.size();
      camera->getImage()->setPixel(dx, dy, L);
    }
  }
}

Vec3f PhongLightingIntegrator::radiance(Ray &ray, Interaction &interaction, LightSample light) const
{
  Vec3f radiance(0, 0, 0);
  // TODO: Your code here.

  // Ambient
  Vec3f ambient = this->scene->getAmbient();

  // Diffuse
  Vec3f norm_dir = interaction.normal.normalized();
  Vec3f light_dir = (light.position - interaction.pos).normalized(); // obj2light
  float diffuse = norm_dir.dot(light_dir);
  if (diffuse < 0)
    diffuse = 0.0f;

  // specular
  // reflect
  Vec3f AO = -light_dir;
  Vec3f PO = -norm_dir;
  Vec3f BO = 2 * (AO.dot(PO)) * PO - AO;
  Vec3f view_dir = -ray.direction; // viewpos - fragpos
  float specular = std::pow(std::max(view_dir.dot(-BO), 0.0f), interaction.model.shininess);
  // specular = 0;

  Vec3f light_coeff;
  light_coeff = diffuse * interaction.model.diffusion;
  light_coeff += specular * interaction.model.specular;
  radiance = dot_vector_(ambient, interaction.model.ambient);
  radiance += dot_vector_(light_coeff, light.color);

  return radiance;
}
