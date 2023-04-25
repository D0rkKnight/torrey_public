/**
 * Represents a ray in 3D space.
 */
#include "ray.h"

using namespace cu_utils;
Ray::Ray(Vector3 origin, Vector3 dir)
    : origin(origin), dir(normalize(dir))
{
}

// At operation
Vector3 Ray::operator*(Real t) const
{
    return origin + dir * t;
}

/**
 * @brief Construct a new Ray Hit object
 *
 */
RayHit::RayHit() : hit(false), t(-1), sphere(nullptr), normal(Vector3{0, 0, 0}) {}
RayHit::RayHit(bool hit, Real t, const Shape *sphere, Vector3 normal) : hit(hit), t(t), sphere(sphere), normal(normal) {}