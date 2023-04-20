/**
 * Represents a ray in 3D space.
 */
#pragma once

#include "utils.h"
#include "../vector.h"
#include "scene.h"

namespace cu_utils
{
    class Ray
    {
    public:
        Ray(Vector3 origin, Vector3 dir)
            : origin(origin), dir(normalize(dir))
        {
        }

        Vector3 origin;
        Vector3 dir; // dir should be normalized

        // At operation
        Vector3 operator*(Real t) const
        {
            return origin + dir * t;
        }
    };

    /**
     * @brief Contains info of whether a ray hit a sphere and the t value of the hit.
     * And the normal of the hit, as well.
     *
     */
    class RayHit
    {
    public:
        // Def constructor used for when nothing is hit
        RayHit() : hit(false), t(-1), sphere(nullptr), normal(Vector3{0, 0, 0}) {}
        RayHit(bool hit, Real t, const Shape *sphere, Vector3 normal) : hit(hit), t(t), sphere(sphere), normal(normal) {}

        bool hit;
        Real t;
        const Shape *sphere;
        Vector3 normal;
    };
}