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
    struct Scene;

    struct Shape
    {
    public:
        int material_id;
        const Scene *scene; // The scene that this shape is in
        const AreaLight *areaLight; // The area light that this shape is

        virtual RayHit checkHit(const Ray &ray, const Real mint, const Real maxt) const = 0;
        virtual BoundingBox getBoundingBox() const = 0;
        virtual Ray sampleSurface(int samples, Real &jacobian, pcg32_state &rng) const;
        virtual Real pdfSurface(const Ray &ray) const = 0;
    };

    struct Sphere : public Shape
    {
    public:
        Vector3 center;
        Real radius;

        Sphere(Vector3 center, Real radius, int material_id);
        RayHit checkHit(const Ray &ray, const Real mint, const Real maxt) const override;
        BoundingBox getBoundingBox() const override;
        Ray sampleSurface(int samples, Real &jacobian, pcg32_state &rng) const override;
        Real pdfSurface(const Ray &ray) const override;
    };

    struct Triangle : public Shape
    {
    public:
        Vector3 v0, v1, v2;
        Vector2 uv0, uv1, uv2;
        Vector3 n0, n1, n2;

        Triangle(Vector3 v0, Vector3 v1, Vector3 v2, int material_id);
        RayHit checkHit(const Ray &ray, const Real mint, const Real maxt) const override;
        BoundingBox getBoundingBox() const override;
        Ray sampleSurface(int samples, Real &jacobian, pcg32_state &rng) const override;
        Real pdfSurface(const Ray &ray) const override;

        // Given a hit, return the barycentric coordinates of the hit
        Vector3 getBarycentric(const Vector3 p) const;

        void setUVs(Vector2 uv0, Vector2 uv1, Vector2 uv2);
    };
}