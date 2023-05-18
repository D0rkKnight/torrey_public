#include <gtest/gtest.h>
#include "../vector.h"
#include "../custom/ray.h"
#include "../custom/bounding_box.h"
#include "../custom/shapes.h"


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
    
    BBNode root = BBNode::buildTree(shapes);
    std::vector<Shape*> shapesInTree;
    std::function<void(const BBNode&)> traverse = [&](const BBNode& node) {
        if (node.shape != nullptr) {
            shapesInTree.push_back(node.shape);
        }
        for (const BBNode& child : node.children) {
            traverse(child);
        }
    };
    traverse(root);
    for (Shape* shape : shapes) {
        EXPECT_TRUE(std::find(shapesInTree.begin(), shapesInTree.end(), shape) != shapesInTree.end());
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}