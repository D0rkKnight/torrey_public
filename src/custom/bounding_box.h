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

    struct BBNode
    {
    public:
        BoundingBox box;
        Shape *shape;
        std::vector<BBNode> children;

        BBNode(BoundingBox box, Shape *shape); // Children are written to afterwards
        RayHit checkHit(const Ray &ray) const;

        static BBNode buildTree(std::vector<Shape *> shapes);
    };
}