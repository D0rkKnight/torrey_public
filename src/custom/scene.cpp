#pragma once
#include "scene.h"
#include "../vector.h"

using namespace cu_utils;
RayHit Shape::checkHit(const Ray &ray) const
{
    // Borrowed from RT in One Weekend
    Vector3 oc = ray.origin - center; // Vector from origin to center of sphere
    auto a = dot(ray.dir, ray.dir);
    auto b = 2 * dot(oc, ray.dir);
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;

    if (discriminant > 0)
    {
        // Return distance to hit point

        // Check the closer hit point first
        Real t = (-b - sqrt(discriminant)) / (2.0 * a);
        Vector3 n = normalize(ray * t - center);

        if (t <= 0)
        {
            // Check the further hit point
            t = (-b + sqrt(discriminant)) / (2.0 * a);

            // normal is pointing the other way in this case
            n = -normalize(ray * t - center);
        }

        // Both potential hit points are behind the camera, invalid.
        if (t <= 0)
            return RayHit();

        // Valid rayhit found, returning
        return RayHit(true, t, this, n);
    }
    else
    {
        return RayHit(); // Missed
    }
};

Scene::Scene()
{
    // Create a scene with a single sphere

    camera = AbstractCamera{Vector3{0, 0, 0}, Vector3{0, 0, 1}, Vector3{0, 1, 0}, (Real)90.0};

    // Create a sphere
    shapes = std::vector<Shape>();
    shapes.push_back(Shape{Vector3{0, 0, 2}, (Real)1.0, 0});

    // Create materials
    materials = std::vector<Material>();
    materials.push_back(Material{MaterialType::Diffuse, Vector3{1.0, 0.5, 0.5}});
}