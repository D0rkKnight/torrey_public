#include <gtest/gtest.h>
#include "../vector.h"
#include "../custom/ray.h"
#include "../custom/bounding_box.h"
#include "../custom/shapes.h"
#include "../custom/utils.h"


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

