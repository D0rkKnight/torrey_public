#pragma once

#include <iostream>
#include <vector>
#include "../vector.h"
#include "../parse_scene.h"
#include "pcg.h"
#include "ray.h"
#include "bounding_box.h"

namespace cu_utils
{
    class AreaLight;

    struct Shape
    {
    public:
        int material_id;
        const AreaLight *areaLight; // The area light that this shape is

        virtual RayHit checkHit(const Ray &ray) const = 0;
        virtual BoundingBox getBoundingBox() const = 0;
        virtual std::vector<Ray> sampleSurface(int samples, std::vector<Real> &jacobians, pcg32_state &rng) const;
    };

    struct Sphere : public Shape
    {
    public:
        Vector3 center;
        Real radius;

        Sphere(Vector3 center, Real radius, int material_id);
        RayHit checkHit(const Ray &ray) const override;
        BoundingBox getBoundingBox() const override;
        std::vector<Ray> sampleSurface(int samples, std::vector<Real> &jacobians, pcg32_state &rng) const override;
    };

    struct Triangle : public Shape
    {
    public:
        Vector3 v0, v1, v2;
        Vector2 uv0, uv1, uv2;
        Vector3 n0, n1, n2;

        Triangle(Vector3 v0, Vector3 v1, Vector3 v2, int material_id);
        RayHit checkHit(const Ray &ray) const override;
        BoundingBox getBoundingBox() const override;
        std::vector<Ray> sampleSurface(int samples, std::vector<Real> &jacobians, pcg32_state &rng) const override;

        // Given a hit, return the barycentric coordinates of the hit
        Vector3 getBarycentric(const Ray &ray, const RayHit &hit) const;

        void setUVs(Vector2 uv0, Vector2 uv1, Vector2 uv2);
    };
}