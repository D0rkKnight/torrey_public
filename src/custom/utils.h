#pragma once
#include "../image.h"
#include "../vector.h"
#include "../matrix.h"
#include "ray.h"
#include "pcg.h"
#include <cstdlib>

namespace cu_utils
{
    // Seems like GCC has M_PI allocated already
    const Real MY_PI = 3.14159265358979323846;

    inline Real d2r(Real deg)
    {
        return deg * MY_PI / 180;
    }

    inline Real r2d(Real rad)
    {
        return rad * 180 / MY_PI;
    }

    inline Vector4 matXvec(const Matrix4x4 &mat, const Vector4 &vec)
    {
        return Vector4{
            mat(0, 0) * vec.x + mat(0, 1) * vec.y + mat(0, 2) * vec.z + mat(0, 3) * vec.w,
            mat(1, 0) * vec.x + mat(1, 1) * vec.y + mat(1, 2) * vec.z + mat(1, 3) * vec.w,
            mat(2, 0) * vec.x + mat(2, 1) * vec.y + mat(2, 2) * vec.z + mat(2, 3) * vec.w,
            mat(3, 0) * vec.x + mat(3, 1) * vec.y + mat(3, 2) * vec.z + mat(3, 3) * vec.w,
        };
    }

    /**
     * @brief For reflecting off of surfaces
     *
     * @param v
     * @param n
     * @return Vector3
     */
    inline Vector3 reflect(const Vector3 &v, const Vector3 &n)
    {
        return v - 2 * dot(v, n) * n;
    }

    inline Vector3 hadamard(const Vector3 &a, const Vector3 &b)
    {
        return Vector3{a.x * b.x, a.y * b.y, a.z * b.z};
    }

    inline Vector3 fresnelSchlick(const Vector3 &f0, const Vector3 &n, const Vector3 &v)
    {
        return f0 + (1 - f0) * pow(1 - dot(n, v), 5);
    }

    /**
     * @brief Returns reflect ray that is offset by epsilon
     *
     */
    inline Ray getBounceRay(Ray ray, RayHit bestHit)
    {
        // Get the reflection direction
        Vector3 hit = ray * bestHit.t;
        Vector3 reflectDir = reflect(ray.dir, bestHit.normal);
        Ray reflectRay = Ray(hit, reflectDir);

        // Move the ray forward by 10^-4
        reflectRay.origin += reflectRay.dir * 0.0001;

        return reflectRay;
    }

    inline bool equals(const Vector3& a, const Vector3& b, float epsilon = 1e-6f) {
        return fabs(a.x - b.x) < epsilon && fabs(a.y - b.y) < epsilon && fabs(a.z - b.z) < epsilon;
    };

    inline int longestExtent (const Vector3& v) {
        if (v.x >= v.y && v.x >= v.z) {
            return 0;
        } else if (v.y >= v.z) {
            return 1;
        } else {
            return 2;
        }
    };

    inline Vector3 randomUnitVector(pcg32_state &rng) {
        Vector3 v;
        do {
            v = Vector3{next_pcg32_real<Real>(rng), next_pcg32_real<Real>(rng), next_pcg32_real<Real>(rng)} * 2.0 - Vector3{1, 1, 1};
        } while (length_squared(v) < 1e-6);

        return normalize(v);
    }

    inline Vector3 random_cosine_direction(pcg32_state &rng) {
        auto r1 = next_pcg32_real<Real>(rng);
        auto r2 = next_pcg32_real<Real>(rng);
        auto z = sqrt(1-r2);

        auto phi = 2*MY_PI*r1;
        auto x = cos(phi)*sqrt(r2);
        auto y = sin(phi)*sqrt(r2);

        return Vector3(x, y, z);
    }
}