#include "integrator.h"
#include "utils.h"
#include <omp.h>

#include <utility>

Integrator::Integrator(std::shared_ptr<Camera> cam,
                       std::shared_ptr<Scene> scene, int spp, int max_depth)
    : camera(std::move(cam)), scene(std::move(scene)), spp(spp), max_depth(max_depth)
{
}

void Integrator::render() const
{
  Vec2i resolution = camera->getImage()->getResolution();
  int cnt = 0;
  Sampler sampler;
#pragma omp parallel for schedule(dynamic), shared(cnt), private(sampler)
  for (int dx = 0; dx < resolution.x(); dx++)
  {
#pragma omp atomic
    ++cnt;
    printf("\r%.02f%%", cnt * 100.0 / resolution.x());
    sampler.setSeed(omp_get_thread_num());
    for (int dy = 0; dy < resolution.y(); dy++)
    {
      Vec3f L(0, 0, 0);
      // TODO: generate #spp rays for each pixel and use Monte Carlo integration to compute radiance.
      for (int i = 0; i < spp; ++i)
      {
        Vec2f delta = sampler.get2D();
        Ray r = camera->generateRay((float)dx + delta.x(), (float)dy + delta.y());
        L += radiance(r, sampler); // TODO
      }
      L /= spp;
      camera->getImage()->setPixel(dx, dy, L);
    }
  }
}

Vec3f Integrator::radiance(Ray &ray, Sampler &sampler) const
{
  Vec3f L(0, 0, 0);
  Vec3f beta(1, 1, 1);
  bool isDelta = false;
  for (int i = 0; i < max_depth; ++i)
  {
    /// Compute radiance (direct + indirect)
    Interaction interaction;
    bool intersect = scene->intersect(ray, interaction);
    interaction.wi = ray.direction;
    if (intersect)
    {
      // assert(interaction.material->isDelta() == false); // all reflection
      // printf("faq type = %d, %d\n", interaction.type, interaction.material->isDelta());
      if (interaction.type == Interaction::Type::LIGHT && i == 0)
      {
        L = scene->getLight()->emission(interaction.pos, -interaction.wi); // direct
        break;
      }
      else if (interaction.type == Interaction::Type::GEOMETRY && interaction.material->isDelta())
      {
        // translucent, specular
        float pdf = interaction.material->sample(interaction, sampler); //1.0f
        Vec3f _brdf = interaction.material->evaluate(interaction);

        ray = Ray(interaction.pos + 1e-5 * interaction.wo, interaction.wo);
        Interaction interaction_2;
        bool ref_intersection = scene->intersect(ray, interaction_2);
        interaction_2.wi = ray.direction; //wo

        if (ref_intersection && interaction_2.type == Interaction::Type::LIGHT && i == 0)
        {
          // if (ref_intersection && interaction_2.type == Interaction::Type::LIGHT) {
          Vec3f _light = scene->getLight()->emission(interaction_2.pos, -interaction_2.wi);
          L += beta.cwiseProduct(_light.cwiseProduct(_brdf));
          break;
        }
        beta = beta.cwiseProduct(_brdf) / 1.f; //specular
      }
      else if (interaction.type == Interaction::Type::GEOMETRY && !interaction.material->isDelta())
      {
        // reflection
        //Direct Lighting
        Vec3f direct_light = directLighting(interaction, sampler);
        L += beta.cwiseProduct(direct_light);
        //Sample next ray
        float pdf = interaction.material->sample(interaction, sampler);
        float cos_theta = interaction.wo.dot(interaction.normal); //must > 0
        Vec3f brdf = interaction.material->evaluate(interaction);
        //Update beta
        beta = beta.cwiseProduct(brdf) * cos_theta / pdf;
        //Spawn and march the new ray
        ray = Ray(interaction.pos + 1e-5 * interaction.wo, interaction.wo);
      }
    }
    else
    {
      assert(false);
      break;
    }
  }
  return L;
}

Vec3f Integrator::directLighting(Interaction &interaction, Sampler &sampler) const
{
  Vec3f L(0, 0, 0);
  // Compute direct lighting.
  float pdf_light;
  Vec3f pos;
  std::shared_ptr<Light> light = scene->getLight();
  pos = light->sample(interaction, &pdf_light, sampler);
  Ray rayray(interaction.pos + 1e-5 * interaction.wo, interaction.wo);

  if (!scene->isShadowed(rayray))
  {
    Vec3f _light = light->emission(pos, -interaction.wo);
    Vec3f _brdf = interaction.material->evaluate(interaction);
    float cos_theta = interaction.wo.dot(interaction.normal);
    float cosDivSquDist = light->pdf(interaction, pos);
    L = _light.cwiseProduct(_brdf) * cos_theta * cosDivSquDist / pdf_light;
  }
  return L;
}