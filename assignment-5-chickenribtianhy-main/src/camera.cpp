#include "camera.h"
#include "cloth.h"
Camera::Camera(Float fov_y) : fov_y(fov_y)
{
  tan_half_fov_y = Float(tanf(fov_y / 2.f * pi / 180.f));
}

Mat4 Camera::LookAtMat() const
{
  return glm::lookAt(transform.position, transform.position + transform.Forward(), Vec3(0, 1, 0));
}

Mat4 Camera::PerspectiveMat() const
{
  return glm::perspective(glm::radians(fov_y), aspect, near, far);
}

void Camera::set_cloth(const std::shared_ptr<RectCloth> c)
{
  cloth = c;
}

Vec3 last_near = {-1, -1, -1};
Float last_ratio = -1;
int fixed_mass = -1;
// Float max_dist = 1e8;
#define CMP_TOLERANCE 1e-3

void Camera::Update()
{
  aspect = Float(Input::window_width) / Float(Input::window_height);

  bool is_clicked = Input::GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
  if (is_clicked)
  {
    Float mouse_x = Input::mouse_position.x;
    Float mouse_y = Input::mouse_position.y;
    Float mouse_limit_x = (Float)Input::window_width;
    Float mouse_limit_y = (Float)Input::window_height;
    Vec3 new_near = {0, 0, 0};
    // camera coordinate : mouse on near plane
    // Vec3 x_near = (mouse_x / mouse_limit_x - 0.5f) * 2 * near * tan_half_fov_y * transform.Right() * aspect;
    // Vec3 y_near = (mouse_y / mouse_limit_y - 0.5f) * 2 * near * tan_half_fov_y * transform.Up();
    // Vec3 z_near = near * transform.Forward();
    new_near += (mouse_x / mouse_limit_x - 0.5f) * 2 * near * tan_half_fov_y * transform.Right() * aspect;
    new_near += (mouse_y / mouse_limit_y - 0.5f) * 2 * near * tan_half_fov_y * transform.Up();
    new_near += near * transform.Forward();
    // ray from camera to new_near(world)
    Vec3 ray_direction = new_near;
    // from camsera coordinate to world coordinate
    new_near += transform.position;

    if (fixed_mass == -1)
    {
      Float max_dist = 1e8;
      // update fixed_mass
      // fixed_mass = 10;
      // enumerate to find the mass
      cloth->LocalToWorldPositions();
      // world
#pragma omp parallel for collapse(2)
      for (int ih = 0; ih < cloth->mass_dim.y; ++ih)
      {
        for (int iw = 0; iw < cloth->mass_dim.x; ++iw)
        {
          int _index = cloth->Get1DIndex(iw, ih);
          Vec3 _enum_cloth_world_pos = cloth->local_or_world_positions[_index];
          Vec3 ray_direction_enum_cloth = _enum_cloth_world_pos - transform.position;
          Float _cos_2_rays = glm::dot(glm::normalize(ray_direction), glm::normalize(ray_direction_enum_cloth));
          if ((1 - _cos_2_rays) * glm::length(ray_direction_enum_cloth) < CMP_TOLERANCE)
          {
            // targeted
            if ((1 - _cos_2_rays) * glm::length(ray_direction_enum_cloth) < max_dist)
            {
              max_dist = (1 - _cos_2_rays) * glm::length(ray_direction_enum_cloth);
              fixed_mass = cloth->Get1DIndex(iw, ih);
            }
          }
        }
      }
      cloth->WorldToLocalPositions();
      // local
      if (fixed_mass != -1)
      {
        // found a mass
        last_near = new_near;
        cloth->is_fixed_masses[fixed_mass] = true;
        cloth->world_accelerations[fixed_mass] = {0, 0, 0};
        cloth->world_velocities[fixed_mass] = {0, 0, 0};
        last_ratio = glm::length(ray_direction) /
                     glm::length(cloth->local_or_world_positions[fixed_mass] - transform.position);
      }
    }
    else
    {
      // already a fixed mass
      Vec3 diff = new_near - last_near;
      last_near = new_near;
      cloth->LocalToWorldPositions();
      cloth->local_or_world_positions[fixed_mass] += diff / last_ratio;
      cloth->WorldToLocalPositions();
    }
    return;
  }
  else
  {
    // not clicked
    if (fixed_mass != -1)
      cloth->is_fixed_masses[fixed_mass] = false;
    fixed_mass = -1;
  }

#ifdef FIRST_PERSON_CAMERA
  Input::SetCursorVisibility(true);
#endif
  int get_w = int(Input::GetKey(KeyCode::W));
  int get_a = int(Input::GetKey(KeyCode::A));
  int get_s = int(Input::GetKey(KeyCode::S));
  int get_d = int(Input::GetKey(KeyCode::D));
  int get_q = int(Input::GetKey(KeyCode::Q));
  int get_e = int(Input::GetKey(KeyCode::E));
  is_speeding = get_w + get_a + get_s + get_d + get_q + get_e > 0;
  if (is_speeding)
    speeding_rate = std::lerp(speeding_rate, one, ACCELERATION * Time::fixed_delta_time);
  else
    speeding_rate = std::lerp(speeding_rate, zero, DECELERATION * Time::fixed_delta_time);
  float moveSpeed =
      Input::GetKey(KeyCode::LeftShift) || Input::GetKey(KeyCode::RightShift) ? MOVE_SPEED_FAST : MOVE_SPEED_SLOW;
  // WASD
  Float movement = speeding_rate * moveSpeed * (Float)Time::delta_time;
  transform.position += transform.Forward() * (Float(get_w) * movement);
  transform.position += transform.Left() * (Float(get_a) * movement);
  transform.position += transform.Back() * (Float(get_s) * movement);
  transform.position += transform.Right() * (Float(get_d) * movement);
  // Q/E
  transform.position += Vec3(0, 1, 0) * (Float(get_e) * movement);
  transform.position += Vec3(0, -1, 0) * (Float(get_q) * movement);
  // rotate
#ifndef FIRST_PERSON_CAMERA
  if (Input::GetMouseButton(1))
  {
    Input::SetCursorVisibility(false);
    transform.Rotate(transform.Right(), glm::radians(Input::mouse_position_frame_offset.y * mouse_sensitivity));
    transform.Rotate(Vec3(0, 1, 0), -glm::radians(Input::mouse_position_frame_offset.x * mouse_sensitivity));
  }
  else
  {
    Input::SetCursorVisibility(true);
  }
#else
  // rotate
  transform.Rotate(transform.Right(), glm::radians(Input::mouse_position_frame_offset.y * mouse_sensitivity));
  transform.Rotate(Vec3(0, 1, 0), -glm::radians(Input::mouse_position_frame_offset.x * mouse_sensitivity));
  // clamp
  Vec3 rotation_euler = glm::eulerAngles(transform.rotation);
  float rotation_euler_degX = glm::degrees(rotation_euler.x);
  float inf, sup;
  if (rotation_euler_degX <= -90)
  {
    inf = -180;
    sup = -180 + pitch_max;
  }
  else if (rotation_euler_degX <= 0)
  {
    inf = -pitch_max;
    sup = 0;
  }
  else if (rotation_euler_degX <= 90)
  {
    inf = 0;
    sup = pitch_max;
  }
  else
  { // rotation_euler_degX <= 180
    inf = 180 - pitch_max;
    sup = 180;
  }
  rotation_euler.x = glm::radians(glm::clamp(rotation_euler_degX, inf, sup));
  transform.rotation = Quat(rotation_euler);
#endif
}
