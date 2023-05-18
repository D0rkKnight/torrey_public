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
        bool checkHit(const Ray &ray, Real tmin, Real tmax) const;

        // Returns the union of the two BBs
        BoundingBox operator+(const BoundingBox &other) const;
    };

    struct BVHNode
    {
    public:
        BoundingBox box;
        std::vector<Shape *> shapes;
        std::vector<BVHNode> children;

        BVHNode(BoundingBox box, std::vector<Shape *> shapes); // Children are written to afterwards
        BVHNode(BoundingBox box, std::vector<Shape *> shapes, std::vector<BVHNode> children); // Children are written to afterwards
        BVHNode();

        RayHit checkHit(const Ray &ray) const;

        static BVHNode buildTree(std::vector<Shape *> shapes);

        static int scansMade;
        static int boxesHit;
    };
}