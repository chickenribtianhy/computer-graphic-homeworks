#include "camera.h"

Camera::Camera()
    : position(0, -1, 0), fov(45), focal_len(1)
{
  lookAt({0, 0, 0}, {0, 0, 1});
}

Ray Camera::generateRay(float dx, float dy)
{
  // TODO: Your code here
  // You need to generate ray according to screen coordinate (dx, dy)
  // return Ray(Vec3f(0, 0, 0), Vec3f(0, 0, 0));
  auto img_resolution = image->getResolution();
  auto img_aspect = image->getAspectRatio();
  float px = (dx / img_resolution.x() * 2 - 1) * img_aspect;
  float py = (dy / img_resolution.y() * 2 - 1);

  // float _theta = fov / 2 * PI / 180;
  float _scale = scale;

  Vec3f direction = focal_len * forward; // to focus plane
  direction += px * _scale * right;
  direction += py * _scale * up;
  direction = direction.normalized();

  return Ray(this->position, direction);
}

void Camera::lookAt(const Vec3f &look_at, const Vec3f &ref_up)
{
  // TODO: Your code here
  // forward = (position - look_at).normalized();
  // right = ref_up.cross(forward).normalized();
  // up = forward.cross(right).normalized();

  float _theta = fov / 2 * PI / 180;
  scale = tanf(_theta) * focal_len;

  forward = (look_at - position).normalized();
  right = forward.cross(ref_up).normalized();
  up = right.cross(forward).normalized();
}
void Camera::setPosition(const Vec3f &pos)
{
  position = pos;
}
Vec3f Camera::getPosition() const
{
  return position;
}
void Camera::setFov(float new_fov)
{
  fov = new_fov;
}
float Camera::getFov() const
{
  return fov;
}

void Camera::setImage(std::shared_ptr<ImageRGB> &img)
{
  image = img;
}
std::shared_ptr<ImageRGB> &Camera::getImage()
{
  return image;
}

float Camera::get_focal_len()
{
  return focal_len;
}