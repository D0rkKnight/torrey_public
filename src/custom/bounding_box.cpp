
#include "bounding_box.h"
#include "shapes.h"
#include <iostream>

using namespace cu_utils;

BoundingBox::BoundingBox(Vector3 min, Vector3 max)
{
    this->minc = min;
    this->maxc = max;
}

BoundingBox::BoundingBox()
{
    minc = Vector3{std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max()};
    maxc = Vector3{std::numeric_limits<Real>::lowest(), std::numeric_limits<Real>::lowest(), std::numeric_limits<Real>::lowest()};
}

bool BoundingBox::checkHit(const Ray &ray) const
{
    Vector3 invdir = 1.0 / ray.dir;
    Vector3 t0 = (minc - ray.origin) * invdir;
    Vector3 t1 = (maxc - ray.origin) * invdir;
    Vector3 tmin = min(t0, t1);
    Vector3 tmax = max(t0, t1);
    Real tminmax = std::max(tmin.x, std::max(tmin.y, tmin.z));
    Real tmaxmin = std::min(tmax.x, std::min(tmax.y, tmax.z));

    return tminmax <= tmaxmin;
}

BBNode::BBNode(BoundingBox box, Shape *shape)
{
    this->box = box;
    this->shape = shape;
}

BBNode BBNode::buildTree(std::vector<Shape *> shapes)
{
    // If it's just one shape, return a node with that shape
    if (shapes.size() == 1)
    {
        return BBNode(shapes[0]->getBoundingBox(), shapes[0]);
    }

    // Otherwise, return a branch node
    BoundingBox box;
    bool dimsInitialized = false;

    BBNode root = BBNode(box, nullptr);
    for (int i = 0; i < shapes.size(); i++)
    {
        BoundingBox cbox = shapes[i]->getBoundingBox();

        // Initialize the root bounding box, will be infinite size otherwise
        if (!dimsInitialized)
        {
            root.box = box;
            dimsInitialized = true;
        }

        box.minc = min(box.minc, cbox.minc);
        box.maxc = max(box.maxc, cbox.maxc);
    }

    // Get the longer axis and sort the shapes by that axis
    Vector3 dims = root.box.maxc - root.box.minc;
    int longestAxis = 0;
    if (dims.y > dims.x && dims.y > dims.z)
    {
        longestAxis = 1;
    }
    else if (dims.z > dims.x && dims.z > dims.y)
    {
        longestAxis = 2;
    }

    // Sort the shapes by the longest axis
    std::sort(shapes.begin(), shapes.end(), [longestAxis](Shape *a, Shape *b)
              { return a->getBoundingBox().minc[longestAxis] < b->getBoundingBox().minc[longestAxis]; });

    // Split the shapes into two groups
    std::vector<Shape *> leftShapes(shapes.begin(), shapes.begin() + shapes.size() / 2);
    std::vector<Shape *> rightShapes(shapes.begin() + shapes.size() / 2, shapes.end());

    // Recursively build the tree
    root.children.push_back(buildTree(leftShapes));
    root.children.push_back(buildTree(rightShapes));

    return root;
}

RayHit BBNode::checkHit(const Ray &ray) const
{
    if (!box.checkHit(ray))
    {
        return RayHit();
    }

    if (shape != nullptr)
    {
        return shape->checkHit(ray);
    }

    // Go over every child and compare their rayhit dists
    RayHit bestHit = RayHit();

    for (int i = 0; i < children.size(); i++)
    {
        RayHit hit = children[i].checkHit(ray);
        if (hit.hit && (bestHit.hit == false || hit.t < bestHit.t))
        {
            bestHit = hit;
        }
    }

    return bestHit;
}