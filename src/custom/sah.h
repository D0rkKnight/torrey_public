// #pragma once

// #include <vector>
// #include <algorithm>
// #include "bounding_box.h"

// namespace cu_utils {

//     struct BVHPrimitiveInfo {
//         Shape *primitiveRef;
//         BoundingBox bounds;
//         Vector3 centroid;
//     };

//     struct BucketInfo {
//         int count = 0;
//         BoundingBox bounds;
//     };

// const int NUM_BUCKETS = 12;

// float surfaceArea(const BoundingBox& box) {
//     Vector3 d = box.maxc - box.minc;
//     return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
// }

// float intersectCost(int numPrimitives) {
//     return 1.0f;
// }

// float traversalCost(int numPrimitives) {
//     return 1.0f;
// }

// bool compareCentroid(const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b, int dim) {
//     return a.centroid[dim] < b.centroid[dim];
// }

// /**
//  * Complete BVH algorithm for reference (and to break into test cases)
// */
// BVHNode* buildBVH(std::vector<BVHPrimitiveInfo>& primitiveInfo, int start, int end) {
//     BVHNode* node = new BVHNode();

//     // Expand bounds (will start with bad values)
//     BoundingBox bounds;
//     bool init = false;
//     for (int i = start; i < end; i++) {
//         if (!init) {
//             bounds = primitiveInfo[i].bounds;
//             init = true;
//             continue;
//         }
//         bounds = bounds + primitiveInfo[i].bounds;
//     }
    
//     int numPrimitives = end - start;
//     // if (numPrimitives == 1) {
//     //     node->box = bounds;
//     //     node->children = std::vector<BVHNode>();
//     //     node->shape = primitiveInfo[start].primitiveRef;
//     //     return node;
//     // }

//     // // 0 SA node (leaf)
//     // if (bounds.minc[dim] == bounds.maxc[dim]) {
//     //         node->bounds = bounds;
//     //         node->left = nullptr;
//     //         node->right = nullptr;
//     //         for (int i = start; i < end; i++) {
//     //             node->primitives.push_back(primitiveInfo[i].primitiveRef);
//     //         }
//     //         return node;
//     //     }
    
//     int mid = (start + end) / 2;

//     auto maxDim = [](const Vector3& v) {
//         if (v.x > v.y && v.x > v.z) {
//             return 0;
//         } else if (v.y > v.z) {
//             return 1;
//         } else {
//             return 2;
//         }
//     };
//     int dim = maxDim(bounds.maxc - bounds.minc);

//     // if (numPrimitives <= 4) {
//     //     std::nth_element(primitiveInfo.begin() + start, primitiveInfo.begin() + mid, primitiveInfo.begin() + end, [&](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
//     //         return compareCentroid(a, b, dim);
//     //     });
//     // }
//     BucketInfo buckets[NUM_BUCKETS];
//     for (int i = start; i < end; i++) {
//         int bucketIndex = NUM_BUCKETS * ((primitiveInfo[i].centroid[dim] - bounds.minc[dim]) / (bounds.maxc[dim] - bounds.minc[dim]));
//         if (bucketIndex == NUM_BUCKETS) {
//             bucketIndex--;
//         }
//         buckets[bucketIndex].count++;
//         buckets[bucketIndex].bounds = bounds + primitiveInfo[i].bounds;
//     }
//     float cost[NUM_BUCKETS - 1];
//     for (int i = 0; i < NUM_BUCKETS - 1; i++) {
//         BoundingBox b0, b1;
//         Real count0 = 0, count1 = 0;
//         for (int j = 0; j <= i; j++) {
//             b0 = b0 + buckets[j].bounds;
//             count0 += buckets[j].count;
//         }
//         for (int j = i + 1; j < NUM_BUCKETS; j++) {
//             b1 = b1 + buckets[j].bounds;
//             count1 += buckets[j].count;
//         }
//         cost[i] = 0.125f + (count0 * surfaceArea(b0) + count1 * surfaceArea(b1)) / surfaceArea(bounds);
//     }
//     float minCost = cost[0];
//     int minCostSplitBucket = 0;
//     for (int i = 1; i < NUM_BUCKETS - 1; i++) {
//         if (cost[i] < minCost) {
//             minCost = cost[i];
//             minCostSplitBucket = i;
//         }
//     }
//     float leafCost = intersectCost(numPrimitives);
//     if (numPrimitives > 4 && minCost < leafCost) {
//         BVHPrimitiveInfo* midPtr = std::partition(primitiveInfo.data() + start, primitiveInfo.data() + end, [&](const BVHPrimitiveInfo& pi) {
//             int bucketIndex = NUM_BUCKETS * ((pi.centroid[dim] - bounds.minc[dim]) / (bounds.maxc[dim] - bounds.minc[dim]));
//             if (bucketIndex == NUM_BUCKETS) {
//                 bucketIndex--;
//             }
//             return bucketIndex <= minCostSplitBucket;
//         });
//         mid = midPtr - primitiveInfo.data();
//     }
//     else {
//         node->box = bounds;
//         for (int i = start; i < end; i++) {
//             node->primitives.push_back(primitiveInfo[i].primitiveRef);
//         }
//         return node;
//     }

//     node->bounds = bounds;
//     node->left = buildBVH(primitiveInfo, start, mid);
//     node->right = buildBVH(primitiveInfo, mid, end);
//     return node;
// }

// } // namespace cu_utils