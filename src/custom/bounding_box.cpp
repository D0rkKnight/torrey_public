
#include "bounding_box.h"
#include "shapes.h"
#include <iostream>
#include "utils.h"

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

BVHPrimitiveInfo::BVHPrimitiveInfo(Shape *primitiveRef, BoundingBox bounds)
{
    this->primitiveRef = primitiveRef;
    this->bounds = bounds;
    this->centroid = (bounds.minc + bounds.maxc) / 2.0;
}

// Give shapes and precompute bounding boxes
BVHNode BVHNode::buildTree(std::vector<Shape *> shapes) {

    // Generate primitive info
    std::vector<BVHPrimitiveInfo> primInfo;
    for (int i = 0; i < shapes.size(); i++)
    {
        BoundingBox cbox = shapes[i]->getBoundingBox();
        primInfo.push_back(BVHPrimitiveInfo(shapes[i], cbox));
    }

    // Build the tree
    return buildTree(primInfo, 0, shapes.size());
}

BVHNode BVHNode::buildTree(std::vector<BVHPrimitiveInfo> primInfo, int start, int end)
{
    // If it's just one shape, return a node with that shape
    // Wonder if this catches all leaf cases? :/
    if (end - start == 1)
    {
        return BVHNode(primInfo[start].bounds, std::vector<Shape *>{primInfo[start].primitiveRef});
    }

    // Otherwise, return a branch node
    bool dimsInitialized = false;
    BVHNode root = BVHNode(BoundingBox(), std::vector<Shape *>());
    for (int i = start; i < end; i++)
    {
        BoundingBox cbox = primInfo[i].bounds;

        // Initialize the root bounding box, will be infinite size otherwise
        if (!dimsInitialized)
        {
            root.box = cbox;
            dimsInitialized = true;
        }

        // Expand the root bounding box
        root.box = root.box + cbox;
    }

    // Get the longer axis and sort the shapes by that axis
    Vector3 dims = root.box.maxc - root.box.minc;
    int longestAxis = longestExtent(dims);

    // Sort the shapes by the longest axis
    int mid = start + (end - start) / 2;

    std::sort(primInfo.begin()+start, primInfo.begin()+mid , [longestAxis](BVHPrimitiveInfo a, BVHPrimitiveInfo b)
              { return a.bounds.minc[longestAxis] < b.bounds.minc[longestAxis]; });

    // Split the shapes into two groups

    // Recursively build the tree
    root.children.push_back(buildTree(primInfo, start, mid));
    root.children.push_back(buildTree(primInfo, mid, end));

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