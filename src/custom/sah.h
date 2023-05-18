#pragma once

#include <vector>
#include <algorithm>
#include "bounding_box.h"

namespace cu_utils {

    struct BVHPrimitiveInfo {
        Shape *primitiveRef;
        BoundingBox bounds;
        Vector3 centroid;
    };

    struct BucketInfo {
        int count = 0;
        BoundingBox bounds;
    };

    const int NUM_BUCKETS = 12;

    Real surfaceArea(const BoundingBox& box) {
        Vector3 d = box.maxc - box.minc;
        return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    Real intersectCost(int numPrimitives) {
        return 1.0f;
    }

    Real traversalCost(int numPrimitives) {
        return 1.0f;
    }

    bool compareCentroid(const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b, int dim) {
        return a.centroid[dim] < b.centroid[dim];
    }

    int longestExtent (const Vector3& v) {
        if (v.x >= v.y && v.x >= v.z) {
            return 0;
        } else if (v.y >= v.z) {
            return 1;
        } else {
            return 2;
        }
    };

    /**
     * Sorts into NUM_BUCKETS
    */
    void computeBuckets(const std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end, const BoundingBox& bounds, int dim, BucketInfo* buckets, const int NUM_BUCKETS=NUM_BUCKETS) {
        for (int i = start; i < end; i++) {
            int bucketIndex = NUM_BUCKETS * ((primitiveInfo[i].centroid[dim] - bounds.minc[dim]) / (bounds.maxc[dim] - bounds.minc[dim]));
            if (bucketIndex == NUM_BUCKETS) {
                bucketIndex--;
            }
            buckets[bucketIndex].count++;
            buckets[bucketIndex].bounds = bounds + primitiveInfo[i].bounds;
        }
    };

    Real computeBucketCost(const BucketInfo* buckets, int splitBucket, const BoundingBox& bounds, const int NUM_BUCKETS=NUM_BUCKETS) {
        BoundingBox b0, b1;
        Real count0 = 0, count1 = 0;

        // Set some modest values for b0 and b1 in case they don't get unioned at all
        b0 = BoundingBox(Vector3(0, 0, 0), Vector3(0, 0, 0));
        b1 = BoundingBox(Vector3(0, 0, 0), Vector3(0, 0, 0));

        bool initialized = false;
        for (int j = 0; j <= splitBucket; j++) {
            if (!initialized) {
                b0 = buckets[j].bounds;
                count0 = buckets[j].count;
                initialized = true;
                continue;
            }
            b0 = b0 + buckets[j].bounds;
            count0 += buckets[j].count;
        }

        initialized = false;
        for (int j = splitBucket + 1; j < NUM_BUCKETS; j++) {
            if (!initialized) {
                b1 = buckets[j].bounds;
                count1 = buckets[j].count;
                initialized = true;
                continue;
            }

            b1 = b1 + buckets[j].bounds;
            count1 += buckets[j].count;
        }
        return 0.125f + (count0 * surfaceArea(b0) + count1 * surfaceArea(b1)) / surfaceArea(bounds);
    };

    int partitionPrimitives(std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end, const BoundingBox& bounds, int dim, int minCostSplitBucket, const int NUM_BUCKETS=NUM_BUCKETS) {
        auto partitionFunc = [&](const BVHPrimitiveInfo& pi) {
            int bucketIndex = NUM_BUCKETS * ((pi.centroid[dim] - bounds.minc[dim]) / (bounds.maxc[dim] - bounds.minc[dim]));
            if (bucketIndex == NUM_BUCKETS) {
                bucketIndex--;
            }
            
            return bucketIndex <= minCostSplitBucket;
        };
        BVHPrimitiveInfo* midPtr = std::partition(primitiveInfo.data() + start, primitiveInfo.data() + end, partitionFunc);
        return midPtr - primitiveInfo.data();
    }

    /**
     * Complete BVH algorithm for reference (and to break into test cases)
    */
    BVHNode* buildBVH(std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end) {
        BVHNode* node = new BVHNode();

        // Expand bounds (will start with bad values)
        BoundingBox bounds;
        bool init = false;
        for (int i = start; i < end; i++) {
            if (!init) {
                bounds = primitiveInfo[i].bounds;
                init = true;
                continue;
            }
            bounds = bounds + primitiveInfo[i].bounds;
        }
        
        int numPrimitives = end - start;
        // if (numPrimitives == 1) {
        //     node->box = bounds;
        //     node->children = std::vector<BVHNode>();
        //     node->shape = primitiveInfo[start].primitiveRef;
        //     return node;
        // }

        // // 0 SA node (leaf)
        // if (bounds.minc[dim] == bounds.maxc[dim]) {
        //         node->bounds = bounds;
        //         node->left = nullptr;
        //         node->right = nullptr;
        //         for (int i = start; i < end; i++) {
        //             node->primitives.push_back(primitiveInfo[i].primitiveRef);
        //         }
        //         return node;
        //     }
        
        int mid = (start + end) / 2;
        int dim = longestExtent(bounds.maxc - bounds.minc);

        // if (numPrimitives <= 4) {
        //     std::nth_element(primitiveInfo.begin() + start, primitiveInfo.begin() + mid, primitiveInfo.begin() + end, [&](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
        //         return compareCentroid(a, b, dim);
        //     });
        // }
        BucketInfo buckets[NUM_BUCKETS];
        computeBuckets(primitiveInfo, start, end, bounds, dim, buckets);

        Real cost[NUM_BUCKETS - 1];
        for (int i = 0; i < NUM_BUCKETS - 1; i++) {
            cost[i] = computeBucketCost(buckets, i, bounds);
        }

        Real minCost = cost[0];
        int minCostSplitBucket = 0;
        for (int i = 1; i < NUM_BUCKETS - 1; i++) {
            if (cost[i] < minCost) {
                minCost = cost[i];
                minCostSplitBucket = i;
            }
        }

        Real leafCost = intersectCost(numPrimitives);
        if (numPrimitives > 4 && minCost < leafCost) {
            partitionPrimitives(primitiveInfo, start, end, bounds, dim, minCostSplitBucket);
        }
        else {
            // Dump to one node
            node->box = bounds;
            for (int i = start; i < end; i++) {
                node->shapes.push_back(primitiveInfo[i].primitiveRef);
            }
            return node;
        }

        node->box = bounds;
        node->children = {
            buildBVH(primitiveInfo, start, mid), 
            buildBVH(primitiveInfo, mid, end)
        };
        return node;
    };

} // namespace cu_utils