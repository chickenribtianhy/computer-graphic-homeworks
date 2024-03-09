#include "geometry.h"

#include <utility>
#include <iostream>

// TODO: your implementation
const int n = 4; // triangles per leaf node
bool cmp_x(const Triangle &a, const Triangle &b) { return a.bary.x() < b.bary.x(); }
bool cmp_y(const Triangle &a, const Triangle &b) { return a.bary.y() < b.bary.y(); }
bool cmp_z(const Triangle &a, const Triangle &b) { return a.bary.z() < b.bary.z(); }

TriangleMesh::TriangleMesh(std::vector<Vec3f> vertices, std::vector<Vec3f> normals,
                           std::vector<int> v_index, std::vector<int> n_index) : vertices(std::move(vertices)),
                                                                                 normals(std::move(normals)),
                                                                                 v_indices(std::move(v_index)),
                                                                                 n_indices(std::move(n_index)),
                                                                                 bvh(nullptr) {}
// TODO: your implementation
bool TriangleMesh::intersectOneTriangle(Ray &ray, Interaction &interaction, const Vec3i &v_idx, const Vec3i &n_idx) const
{
  Vec3f v0 = vertices[v_idx[0]];
  Vec3f v1 = vertices[v_idx[1]];
  Vec3f v2 = vertices[v_idx[2]];
  Vec3f e1 = v1 - v0;
  Vec3f e2 = v2 - v0;
  Vec3f s1 = ray.direction.cross(e2);
  float det = e1.dot(s1);

  float invDet = 1.0f / det;

  Vec3f s2 = ray.origin - v0;
  float u = s2.dot(s1) * invDet;
  if (u < 0 || u > 1)
    return false;
  Vec3f qvec = s2.cross(e1);
  float v = ray.direction.dot(qvec) * invDet;
  if (v < 0 || u + v > 1)
    return false;
  float t = e2.dot(qvec) * invDet;
  if (t < ray.t_min || t > ray.t_max)
    return false;
  if (t < interaction.dist)
  {
    interaction.dist = t;
    interaction.pos = ray(t);
    interaction.normal = (u * normals[n_idx[1]] + v * normals[n_idx[2]] + (1 - u - v) * normals[n_idx[0]]).normalized();
    interaction.material = bsdf;
    interaction.type = Interaction::Type::GEOMETRY;
    return true;
  }
  return false;
}
// TODO: your implementation
bool TriangleMesh::intersectOneTriangle(Ray &ray, Interaction &interaction, const Triangle &triangle) const
{
  Vec3f v0 = triangle.p0;
  Vec3f v1 = triangle.p1;
  Vec3f v2 = triangle.p2;
  Vec3f e1 = v1 - v0;
  Vec3f e2 = v2 - v0;
  Vec3f s1 = ray.direction.cross(e2);
  float det = e1.dot(s1);

  float invDet = 1.0f / det;

  Vec3f s2 = ray.origin - v0;
  float u = s2.dot(s1) * invDet;
  if (u < 0 || u > 1)
    return false;

  Vec3f qvec = s2.cross(e1);
  float v = ray.direction.dot(qvec) * invDet;
  if (v < 0 || u + v > 1)
    return false;

  float t = e2.dot(qvec) * invDet;
  if (t < ray.t_min || t > ray.t_max)
    return false;
  if (t < interaction.dist)
  {
    interaction.dist = t;
    interaction.pos = ray(t);
    interaction.normal = (u * triangle.n1 + v * triangle.n2 + (1 - u - v) * triangle.n0).normalized();
    interaction.material = bsdf;
    interaction.type = Interaction::Type::GEOMETRY;
    return true;
  }
  return false;
}

bool TriangleMesh::intersect(Ray &ray, Interaction &interaction) const
{
  if (bvh != nullptr)
  {
    bvhHit(bvh, interaction, ray);
  }
  else
  {
    // If you did not implement BVH
    // directly loop through all triangles in the mesh and test intersection for each triangle.
    for (int i = 0; i < v_indices.size() / 3; i++)
    {
      Vec3i v_idx(v_indices[3 * i], v_indices[3 * i + 1], v_indices[3 * i + 2]);
      Vec3i n_idx(n_indices[3 * i], n_indices[3 * i + 1], n_indices[3 * i + 2]);
      intersectOneTriangle(ray, interaction, v_idx, n_idx);
    }
  }
  return interaction.type != Interaction::Type::NONE;
}

void TriangleMesh::setMaterial(std::shared_ptr<BSDF> &new_bsdf)
{
  bsdf = new_bsdf;
}
// TODO
BVHNode *TriangleMesh::_buildBVH_(int l, int r)
{
  if (l > r)
  {
    return nullptr;
  }

  AABB tmp;
  BVHNode *node = new BVHNode();
  node->aabb = AABB();

  for (int i = l; i < r + 1; ++i)
  {
    tmp = AABB(triangles[i].p0, triangles[i].p1, triangles[i].p2);
    node->aabb = AABB(node->aabb, tmp);
  }

  if ((r - l + 1) <= n)
  {
    /* leaf node */
    node->index_l = l;
    node->size = r + 1 - l;
    return node;
  }
  /* internal node */
  float best_cost = 1e9;
  int best_dim = 0;
  int best_split = l + (r - l) / 2;

  for (int dim = 0; dim < 3; dim++)
  {
    if (dim == 0)
      std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmp_x);
    if (dim == 1)
      std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmp_y);
    if (dim == 2)
      std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmp_z);

    std::vector<AABB> left(r - l + 1, AABB());
    left[0] = AABB(triangles[l].p0, triangles[l].p1, triangles[l].p2);
    for (int i = l + 1; i < r + 1; ++i)
    {
      tmp = AABB(triangles[i].p0, triangles[i].p1, triangles[i].p2);
      left[i - l] = AABB(tmp, left[i - l - 1]);
    }

    std::vector<AABB> right(r - l + 1, AABB());
    right[r - l] = AABB(triangles[r].p0, triangles[r].p1, triangles[r].p2);
    for (int i = r - 1; i >= l; --i)
    {
      tmp = AABB(triangles[i].p0, triangles[i].p1, triangles[i].p2);
      right[i - l] = AABB(tmp, right[i - l + 1]);
    }

    float cost = 1e9;
    int split = l;
    for (int i = l; i <= r - 1; ++i)
    {
      tmp = left[i - l];
      float left_S = 2.f * (tmp.getDist(0) * tmp.getDist(1) + tmp.getDist(0) * tmp.getDist(2) + tmp.getDist(1) * tmp.getDist(2));
      float left_cost = left_S * (i - l + 1);

      tmp = right[i + 1 - l];
      float right_S = 2.f * (tmp.getDist(0) * tmp.getDist(1) + tmp.getDist(0) * tmp.getDist(2) + tmp.getDist(1) * tmp.getDist(2));
      float right_cost = right_S * (r - i);

      if (left_cost + right_cost < cost)
      {
        cost = left_cost + right_cost;
        split = i;
      }
    }

    if (cost < best_cost)
    {
      best_cost = cost;
      best_dim = dim;
      best_split = split;
    }
  }

  if (best_dim == 0)
    std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmp_x);
  if (best_dim == 1)
    std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmp_y);
  if (best_dim == 2)
    std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmp_z);

  node->left = _buildBVH_(l, best_split);
  node->right = _buildBVH_(best_split + 1, r);

  return node;
}
void TriangleMesh::buildBVH()
{
  // TODO: your implementation
  for (int i = 0; i < v_indices.size() / 3; i++)
  {
    Triangle tri(vertices[v_indices[3 * i]], vertices[v_indices[3 * i + 1]], vertices[v_indices[3 * i + 2]],
                 normals[n_indices[3 * i]], normals[n_indices[3 * i + 1]], normals[n_indices[3 * i + 2]]);
    triangles.push_back(tri);
  }
  bvh = _buildBVH_(0, triangles.size() - 1);
}

void TriangleMesh::bvhHit(BVHNode *p, Interaction &interaction,
                          Ray &ray) const
{
  // TODO: traverse through the bvh and do intersection test efficiently.
  if (p == nullptr)
    return;

  if (p->size > 0) // leaf
  {
    for (int i = p->index_l; i < p->index_l + p->size; ++i)
    {
      intersectOneTriangle(ray, interaction, triangles[i]);
    }
    return;
  }

  float left_t_in, left_t_out;
  float right_t_in, right_t_out;
  bool left_intersect = false;
  bool right_intersect = false;

  if (p->left)
    left_intersect = p->left->aabb.intersect(ray, &left_t_in, &left_t_out);
  if (p->right)
    right_intersect = p->right->aabb.intersect(ray, &right_t_in, &right_t_out);

  if (left_intersect)
    bvhHit(p->left, interaction, ray);
  if (right_intersect)
    bvhHit(p->right, interaction, ray);
  return;
}
