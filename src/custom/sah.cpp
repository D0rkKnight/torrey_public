#include "sah.h";

using namespace cu_utils;

const int NUM_BUCKETS = 12;

Real cu_utils::surfaceArea(const BoundingBox& box) {
    Vector3 d = box.maxc - box.minc;
    return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
}

Real cu_utils::intersectCost() {
    return 1;
}

Real cu_utils::traversalCost() {
    return 0.125f;
}

bool cu_utils::compareCentroid(const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b, int dim) {
    return a.centroid[dim] < b.centroid[dim];
}

/**
 * Sorts into NUM_BUCKETS
*/
void cu_utils::computeBuckets(const std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end, const BoundingBox& bounds, int dim, BucketInfo* buckets, const int NUM_BUCKETS) {
    for (int i = start; i < end; i++) {
        int bucketIndex = NUM_BUCKETS * ((primitiveInfo[i].centroid[dim] - bounds.minc[dim]) / (bounds.maxc[dim] - bounds.minc[dim]));
        if (bucketIndex == NUM_BUCKETS) {
            bucketIndex--;
        }

        if (!buckets[bucketIndex].initialized) {
            buckets[bucketIndex].bounds = primitiveInfo[i].bounds;
            buckets[bucketIndex].initialized = true;
        } else {
            buckets[bucketIndex].bounds = bounds + primitiveInfo[i].bounds;
        }

        buckets[bucketIndex].count++;
    }
};

Real cu_utils::computeBucketCost(const BucketInfo buckets[], int splitBucket, const BoundingBox& bounds, const int NUM_BUCKETS) {
    BoundingBox b0, b1;
    Real count0 = 0, count1 = 0;

    // Set some modest values for b0 and b1 in case they don't get unioned at all
    b0 = BoundingBox(Vector3(0, 0, 0), Vector3(0, 0, 0));
    b1 = BoundingBox(Vector3(0, 0, 0), Vector3(0, 0, 0));

    bool initialized = false;
    for (int j = 0; j <= splitBucket; j++) {
        if (!buckets[j].initialized) continue;

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
        if (!buckets[j].initialized) continue;

        if (!initialized) {
            b1 = buckets[j].bounds;
            count1 = buckets[j].count;
            initialized = true;
            continue;
        }

        b1 = b1 + buckets[j].bounds;
        count1 += buckets[j].count;
    }
    Real cost = traversalCost() + (intersectCost() * count0 * surfaceArea(b0) + intersectCost() * count1 * surfaceArea(b1)) / surfaceArea(bounds);

    return cost;
};

int cu_utils::partitionPrimitives(std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end, const BoundingBox& bounds, int dim, int minCostSplitBucket, const int NUM_BUCKETS) {
    auto partitionFunc = [&](const BVHPrimitiveInfo& pi) {
        int bucketIndex = NUM_BUCKETS * ((pi.centroid[dim] - bounds.minc[dim]) / (bounds.maxc[dim] - bounds.minc[dim]));
        if (bucketIndex == NUM_BUCKETS) {
            bucketIndex--;
        }
        
    // Equivalent bucket index gets placed into lesser partition
        return bucketIndex <= minCostSplitBucket;
    };
    BVHPrimitiveInfo* midPtr = std::partition(primitiveInfo.data() + start, primitiveInfo.data() + end, partitionFunc);
    return midPtr - primitiveInfo.data();
}