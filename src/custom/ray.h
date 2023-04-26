/**
 * Represents a ray in 3D space.
 */
#pragma once

#include "utils.h"
#include "../vector.h"

namespace cu_utils
{
    class Shape; // Forward declaration

    class Ray
    {
    public:
        Ray(Vector3 origin, Vector3 dir);

        Vector3 origin;
        Vector3 dir; // dir should be normalized

        // At operation
        Vector3 operator*(Real t) const;
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
        RayHit();
        RayHit(bool hit, Real t, const Shape *sphere, Vector3 normal);

        bool hit;
        Real t;
        const Shape *sphere;
        Vector3 normal;
    };
}