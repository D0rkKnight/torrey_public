
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
    return checkHit(ray, 0.0, std::numeric_limits<Real>::max());
}

bool BoundingBox::checkHit(const Ray &ray, Real tmin, Real tmax) const
{
    for (int a=0; a<3; a++) {
        Real origin = ray.origin[a];

        auto invdir = 1.0 / ray.dir[a];
        auto t0 = (minc[a] - origin) * invdir;
        auto t1 = (maxc[a] - origin) * invdir;

        if (invdir < 0.0)
            std::swap(t0, t1);

        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;

        if (tmax < tmin)
            return false;
    }

    return true;
}

BoundingBox BoundingBox::operator+(const BoundingBox &other) const {
    Vector3 minc = Vector3{std::min(this->minc.x, other.minc.x), std::min(this->minc.y, other.minc.y), std::min(this->minc.z, other.minc.z)};
    Vector3 maxc = Vector3{std::max(this->maxc.x, other.maxc.x), std::max(this->maxc.y, other.maxc.y), std::max(this->maxc.z, other.maxc.z)};

    return BoundingBox(minc, maxc);    
}

int BVHNode::scansMade = 0;
int BVHNode::boxesHit = 0;

BVHNode::BVHNode(): BVHNode(BoundingBox(), std::vector<Shape *>()) {}

BVHNode::BVHNode(BoundingBox box, std::vector<Shape *> shapes)
{
    this->box = box;
    this->shapes = shapes;
}

BVHNode::BVHNode(BoundingBox box, std::vector<Shape *> shapes, std::vector<BVHNode> children): BVHNode(box, shapes)
{
    this->children = children;
}

BVHNode BVHNode::buildTree(std::vector<Shape *> shapes)
{
    // If it's just one shape, return a node with that shape
    if (shapes.size() == 1)
    {
        return BVHNode(shapes[0]->getBoundingBox(), shapes);
    }

    // Otherwise, return a branch node
    bool dimsInitialized = false;
    BVHNode root = BVHNode(BoundingBox(), std::vector<Shape *>());
    for (int i = 0; i < shapes.size(); i++)
    {
        BoundingBox cbox = shapes[i]->getBoundingBox();

        // Initialize the root bounding box, will be infinite size otherwise
        if (!dimsInitialized)
        {
            root.box = cbox;
            dimsInitialized = true;
        }

        root.box.minc = min(root.box.minc, cbox.minc);
        root.box.maxc = max(root.box.maxc, cbox.maxc);
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

RayHit BVHNode::checkHit(const Ray &ray) const
{
    // Check for bounding box effectiveness.
    // scansMade++; I believe these hurt performance by breaking parallelism

    if (!box.checkHit(ray))
    {
        return RayHit();
    }

    // boxesHit++;

    if (shapes.size() > 0)
    {
        // Go thru shapes and get best t
        RayHit bestHit = RayHit();
        for (int i = 0; i < shapes.size(); i++)
        {
            RayHit hit = shapes[i]->checkHit(ray);
            if (hit.hit && (bestHit.hit == false || hit.t < bestHit.t))
            {
                bestHit = hit;
            }
        }

        return bestHit;
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