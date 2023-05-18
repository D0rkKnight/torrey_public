#pragma once

#include <vector>
#include <algorithm>
#include "bounding_box.h"

namespace cu_utils {

    struct BVHNode {
        BBNode* node;
        BVHNode* left;
        BVHNode* right;
        BVHNode(BBNode* n, BVHNode* l, BVHNode* r) : node(n), left(l), right(r) {}
    };

    struct BVHBuildEntry {
        int start;
        int end;
        BVHNode* parent;
    };

    struct BVHPrimitiveInfo {
        int primitiveNumber;
        BoundingBox bounds;
        Vector3 centroid;
        BVHPrimitiveInfo(int pn, const BoundingBox& b) : primitiveNumber(pn), bounds(b), centroid((b.minc + b.maxc) * 0.5) {}
    };

    struct BVHBuildNode {
        BoundingBox bounds;
        BVHNode* node;
        BVHBuildNode(const BoundingBox& b, BVHNode* n) : bounds(b), node(n) {}
    };

    inline bool compareCentroid(const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b, int dim) {
        return a.centroid[dim] < b.centroid[dim];
    }

    inline bool compareBounds(const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b, int dim) {
        return a.bounds.minc[dim] < b.bounds.minc[dim];
    }

    inline BVHNode* recursiveBuild(std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end, std::vector<BVHBuildNode>& buildNodes) {
        if (start == end) {
            return new BVHNode(primitiveInfo[start].bounds.buildNode(), nullptr, nullptr);
        }
        BoundingBox bounds;
        for (int i = start; i < end; i++) {
            bounds.expand(primitiveInfo[i].bounds);
        }
        int numPrimitives = end - start;
        if (numPrimitives == 1) {
            return new BVHNode(primitiveInfo[start].bounds.buildNode(), nullptr, nullptr);
        }
        else if (numPrimitives == 2) {
            BVHNode* left = new BVHNode(primitiveInfo[start].bounds.buildNode(), nullptr, nullptr);
            BVHNode* right = new BVHNode(primitiveInfo[start + 1].bounds.buildNode(), nullptr, nullptr);
            return new BVHNode(bounds.buildNode(), left, right);
        }
        else {
            int dim = bounds.maxcDimension();
            if (bounds.minc[dim] == bounds.maxc[dim]) {
                return new BVHNode(primitiveInfo[start].bounds.buildNode(), nullptr, nullptr);
            }
            int mid = (start + end) / 2;
            if (numPrimitives <= 4) {
                std::nth_element(primitiveInfo.begin() + start, primitiveInfo.begin() + mid, primitiveInfo.begin() + end, [&](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
                    return compareCentroid(a, b, dim);
                });
            }
            else {
                std::nth_element(primitiveInfo.begin() + start, primitiveInfo.begin() + mid, primitiveInfo.begin() + end, [&](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
                    return compareBounds(a, b, dim);
                });
            }
            BVHNode* left = recursiveBuild(primitiveInfo, start, mid, buildNodes);
            BVHNode* right = recursiveBuild(primitiveInfo, mid, end, buildNodes);
            return new BVHNode(bounds.buildNode(), left, right);
        }
    }

    inline BVHNode* buildBVH(const std::vector<Shape*>& shapes) {
        std::vector<BVHPrimitiveInfo> primitiveInfo;
        primitiveInfo.reserve(shapes.size());
        for (int i = 0; i < shapes.size(); i++) {
            primitiveInfo.emplace_back(i, shapes[i]->getBoundingBox());
        }
        std::vector<BVHBuildNode> buildNodes;
        buildNodes.reserve(shapes.size() * 2);
        BVHNode* root = recursiveBuild(primitiveInfo, 0, shapes.size(), buildNodes);
        return root;
    }

} // namespace cu_utils