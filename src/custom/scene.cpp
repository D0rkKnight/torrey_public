#pragma once
#include "scene.h"
#include "../vector.h"

using namespace cu_utils;

Scene Scene::defaultScene()
{
    // Create a scene with a single sphere
    Scene scene;

    scene.camera = AbstractCamera{Vector3{0, 0, 0}, Vector3{0, 0, -1}, Vector3{0, 1, 0}, (Real)90.0};

    // Create a sphere
    scene.shapes.push_back(new Sphere(Vector3{0, 0, 2}, (Real)1.0, 0));

    // Create materials
    scene.materials.push_back(Material{MaterialType::Diffuse, Vector3{1.0, 0.5, 0.5}});

    return scene;
}

Scene::Scene()
{
    camera = AbstractCamera{Vector3{0, 0, 0}, Vector3{0, 0, 1}, Vector3{0, -1, 0}, (Real)90.0};
    shapes = std::vector<Shape *>();
    materials = std::vector<Material>();
}