#include <gtest/gtest.h>
#include "../vector.h"
#include "../custom/ray.h"
#include "../custom/bounding_box.h"
#include "../custom/shapes.h"
#include "../custom/utils.h"
#include "../custom/sah.h"


using namespace cu_utils;

// Bounding box tests
TEST(BoundingBoxTest, CheckHitWithSphere) {
    BoundingBox box(Vector3(-1, -1, -1), Vector3(1, 1, 1));
    Ray ray(Vector3(0, 0, -2), Vector3(0, 0, 1));
    EXPECT_TRUE(box.checkHit(ray));

    // Case for box miss
    ray = Ray(Vector3(0, 0, -2), Vector3(0, 1, 0));
    EXPECT_FALSE(box.checkHit(ray));

    // Case for edge shear
    ray = Ray(Vector3(0, 0, -2), Vector3(0, 1, 1));
    EXPECT_TRUE(box.checkHit(ray));

    // Case for corner shear
    ray = Ray(Vector3(0, 0, -2), Vector3(1, 1, 1));
    EXPECT_TRUE(box.checkHit(ray));
}

TEST(BoundingBoxTest, Union) {
    // Test union of two non-overlapping boxes
    BoundingBox box1(Vector3(-1, -1, -1), Vector3(1, 1, 1));
    BoundingBox box2(Vector3(2, 2, 2), Vector3(3, 3, 3));
    BoundingBox result = box1 + box2;
    EXPECT_TRUE(equals(result.minc, Vector3(-1, -1, -1)));
    EXPECT_TRUE(equals(result.maxc, Vector3(3, 3, 3)));

    // Test union of two overlapping boxes
    BoundingBox box3(Vector3(-2, -2, -2), Vector3(0, 0, 0));
    BoundingBox box4(Vector3(-1, -1, -1), Vector3(1, 1, 1));
    result = box3 + box4;
    EXPECT_TRUE(equals(result.minc, Vector3(-2, -2, -2)));
    EXPECT_TRUE(equals(result.maxc, Vector3(1, 1, 1)));

    // Test union of two identical boxes
    BoundingBox box5(Vector3(-1, -1, -1), Vector3(1, 1, 1));
    BoundingBox box6(Vector3(-1, -1, -1), Vector3(1, 1, 1));
    result = box5 + box6;
    EXPECT_TRUE(equals(result.minc, Vector3(-1, -1, -1)));
    EXPECT_TRUE(equals(result.maxc, Vector3(1, 1, 1)));
}

TEST(BVHTest, ContainsAllShapes) {
    std::vector<Shape*> shapes = {
        new Sphere(Vector3(0, 0, 0), 1, 0),
        new Sphere(Vector3(2, 0, 0), 1, 0),
        new Sphere(Vector3(-2, 0, 0), 1, 0),
        new Sphere(Vector3(0, 2, 0), 1, 0),
        new Sphere(Vector3(0, -2, 0), 1, 0),
        new Triangle(Vector3(-1, -1, 0), Vector3(1, -1, 0), Vector3(0, 1, 0), 0),
        new Triangle(Vector3(-2, 0, 0), Vector3(0, -2, 0), Vector3(0, 0, 2), 0),
        new Triangle(Vector3(2, 0, 0), Vector3(0, -2, 0), Vector3(0, 0, 2), 0),
        new Triangle(Vector3(-2, 0, 0), Vector3(0, 2, 0), Vector3(0, 0, 2), 0),
        new Triangle(Vector3(2, 0, 0), Vector3(0, 2, 0), Vector3(0, 0, 2), 0)
    };
    
    BVHNode root = BVHNode::buildTree(shapes);
    std::vector<Shape*> shapesInTree;
    std::function<void(const BVHNode&)> traverse = [&](const BVHNode& node) {
        if (node.shapes.size() >= 0) {
            shapesInTree.insert(shapesInTree.end(), node.shapes.begin(), node.shapes.end());        }
        for (const BVHNode& child : node.children) {
            traverse(child);
        }
    };
    traverse(root);
    for (Shape* shape : shapes) {
        EXPECT_TRUE(std::find(shapesInTree.begin(), shapesInTree.end(), shape) != shapesInTree.end());
    }
}

TEST(SAHandLongestExtentTest, SurfaceArea) {
    // Test surface area of a unit cube
    BoundingBox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
    float result = surfaceArea(box);
    EXPECT_FLOAT_EQ(result, 6.0f);

    // Test surface area of a non-cubic box
    BoundingBox box2(Vector3(-1.0f, -2.0f, -3.0f), Vector3(4.0f, 5.0f, 6.0f));
    float result2 = surfaceArea(box2);
    EXPECT_FLOAT_EQ(result2, 286.0f);
}

TEST(SAHandLongestExtentTest, LongestExtent) {
    // Test longest extent of a unit cube
    BoundingBox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
    int result = longestExtent(box.maxc - box.minc);
    EXPECT_EQ(result, 0);

    // Test longest extent of a non-cubic box
    BoundingBox box2(Vector3(-1.0f, -2.0f, -3.0f), Vector3(4.0f, 5.0f, 6.0f));
    int result2 = longestExtent(box2.maxc - box2.minc);
    EXPECT_EQ(result2, 2);
}

TEST(SAHandPartitionTest, ComputeBuckets) {
    // Test computing buckets for a range of primitives
    std::vector<BVHPrimitiveInfo> primitiveInfo = {
        BVHPrimitiveInfo(nullptr, BoundingBox(Vector3(-3.0f, -2.0f, -2.0f), Vector3(-1.0f, 2.0f, 2.0f))),
        BVHPrimitiveInfo(nullptr, BoundingBox(Vector3(-1.0f, -3.0f, -3.0f), Vector3(1.0f, 3.0f, 3.0f))),
        BVHPrimitiveInfo(nullptr, BoundingBox(Vector3(1.0f, -1.0f, -1.0f), Vector3(3.0f, 1.0f, 1.0f)))
    };
    BoundingBox bounds(Vector3(-3.0f, -3.0f, -3.0f), Vector3(3.0f, 3.0f, 3.0f));

    const int NUM_BUCKETS = 3;
    BucketInfo buckets[NUM_BUCKETS];
    computeBuckets(primitiveInfo, 0, 3, bounds, 0, buckets, NUM_BUCKETS);
    
    EXPECT_EQ(buckets[0].count, 1);
    EXPECT_EQ(buckets[1].count, 1);
    EXPECT_EQ(buckets[2].count, 1);
}

TEST(SAHandPartitionTest, ComputeBucketCost) {
    // Test computing bucket cost for a split
    const int NUM_BUCKETS = 2;
    BucketInfo buckets[NUM_BUCKETS] = {
        {1, BoundingBox(Vector3(-1.0f, -2.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f))},
        {2, BoundingBox(Vector3(-2.0f, 0.0f, -2.0f), Vector3(2.0f, 2.0f, 2.0f))}
    };
    BoundingBox bounds(Vector3(-3.0f, -3.0f, -3.0f), Vector3(3.0f, 3.0f, 3.0f));
    Real cost = computeBucketCost(buckets, 1, bounds, NUM_BUCKETS);
    EXPECT_NEAR(cost, 1.4583f, 0.001);
}

TEST(SAHandPartitionTest, PartitionPrimitives) {
    // Test partitioning primitives based on a split bucket
    std::vector<BVHPrimitiveInfo> primitiveInfo = {
        BVHPrimitiveInfo(nullptr, BoundingBox(Vector3(-1.0f, -1.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f))),
        BVHPrimitiveInfo(nullptr, BoundingBox(Vector3(-1.0f, -2.0f, -2.0f), Vector3(3.0f, 2.0f, 2.0f))),
        BVHPrimitiveInfo(nullptr, BoundingBox(Vector3(-1.0f, -3.0f, -3.0f), Vector3(5.0f, 3.0f, 3.0f)))
    };
    BoundingBox bounds(Vector3(-3.0f, -3.0f, -3.0f), Vector3(3.0f, 3.0f, 3.0f));
    int mid = partitionPrimitives(primitiveInfo, 0, 3, bounds, 0, 7);
    EXPECT_EQ(mid, 1);
}