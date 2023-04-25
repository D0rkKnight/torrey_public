#pragma once

#include "utils.h"
#include "classdef.h"
#include <iostream>
#include <vector>

namespace cu_utils
{
    enum class MaterialType
    {
        Diffuse,
        Mirror
    };

    struct Shape
    {
    public:
        Vector3 center;
        Real radius;
        int material_id;

        RayHit checkHit(const Ray &ray) const;
    };

    struct Material
    {
        MaterialType type;
        Vector3 color;
    };

    struct PointLight
    {
        Vector3 intensity;
        Vector3 position;
    };

    // We don't know the aspect ratio until render time so we can't use a full camera object.
    struct AbstractCamera
    {
        Vector3 lookfrom;
        Vector3 lookat;
        Vector3 up;
        Real vfov;
    };

    struct Scene
    {
        AbstractCamera camera;
        std::vector<Shape> shapes;
        std::vector<Material> materials;
        std::vector<PointLight> lights;

        Scene();
    };
}