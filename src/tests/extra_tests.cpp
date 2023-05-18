#include <gtest/gtest.h>
#include <cmath>

struct Point {
    double x, y, z;
};

struct Sphere {
    Point center;
    double radius;
};

bool intersect(const Sphere& sphere, const Point& point) {
    double distance = std::sqrt(std::pow(point.x - sphere.center.x, 2) +
                                std::pow(point.y - sphere.center.y, 2) +
                                std::pow(point.z - sphere.center.z, 2));
    return distance <= sphere.radius;
}

TEST(SphereTest, IntersectsWithPointInside) {
    Sphere sphere = {{0, 0, 0}, 5};
    Point point = {1, 2, 3};
    EXPECT_TRUE(intersect(sphere, point));
}

TEST(SphereTest, IntersectsWithPointOutside) {
    Sphere sphere = {{0, 0, 0}, 5};
    Point point = {10, 10, 10};
    EXPECT_FALSE(intersect(sphere, point));
}

TEST(SphereTest, IntersectsWithPointOnSurface) {
    Sphere sphere = {{0, 0, 0}, 5};
    Point point = {0, 0, 5};
    EXPECT_TRUE(intersect(sphere, point));
}