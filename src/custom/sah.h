#pragma once

#include <vector>
#include <algorithm>
#include "bounding_box.h"

namespace cu_utils {

    struct BucketInfo {
        int count = 0;
        BoundingBox bounds;
        bool initialized = false;
    };

    const int NUM_BUCKETS = 12;

    Real surfaceArea(const BoundingBox& box);
    Real intersectCost();
    Real traversalCost();

    bool compareCentroid(const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b, int dim);

    /**
     * Sorts into NUM_BUCKETS
    */
    void computeBuckets(const std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end, const BoundingBox& bounds, int dim, BucketInfo* buckets, const int NUM_BUCKETS=NUM_BUCKETS);

    Real computeBucketCost(const BucketInfo buckets[], int splitBucket, const BoundingBox& bounds, const int NUM_BUCKETS=NUM_BUCKETS);

    int partitionPrimitives(std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end, const BoundingBox& bounds, int dim, int minCostSplitBucket, const int NUM_BUCKETS=NUM_BUCKETS);

} // namespace cu_utils