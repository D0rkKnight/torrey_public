#pragma once

#include <iostream>
#include <vector>
#include "../vector.h"
#include "ray.h"

namespace cu_utils
{
    struct BoundingBox
    {
    public:
        Vector3 minc, maxc;

        BoundingBox(Vector3 min, Vector3 max);
        BoundingBox();

        // Returns true if the ray intersects the bounding box
        bool checkHit(const Ray &ray) const;
    };

    struct Shape
    {
    public:
        int material_id;

        virtual RayHit checkHit(const Ray &ray) const = 0;
        virtual BoundingBox getBoundingBox() const = 0;
    };

    struct Sphere : public Shape
    {
    public:
        Vector3 center;
        Real radius;

        Sphere(Vector3 center, Real radius, int material_id);
        RayHit checkHit(const Ray &ray) const override;
        BoundingBox getBoundingBox() const override;
    };

    struct Triangle : public Shape
    {
    public:
        Vector3 v0, v1, v2;

        Triangle(Vector3 v0, Vector3 v1, Vector3 v2, int material_id);
        RayHit checkHit(const Ray &ray) const override;
        BoundingBox getBoundingBox() const override;

        // Given a hit, return the barycentric coordinates of the hit
        Vector3 getBarycentric(const Ray &ray, const RayHit &hit) const;
    };
}