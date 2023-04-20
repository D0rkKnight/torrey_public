#pragma once

#include "utils.h"
#include "../hw1_scenes.h"
#include <iostream>
#include <vector>

namespace cu_utils
{
    enum class MaterialType
    {
        Diffuse,
        Mirror
    };

    struct Sphere
    {
        Vector3 center;
        Real radius;
        int material_id;
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
        std::vector<Sphere> shapes;
        std::vector<Material> materials;
        std::vector<PointLight> lights;

        // Transformer from hw1::Scene to cu_utils::Scene
        Scene(hw1::Scene hw1Scene)
        {
            camera = AbstractCamera{hw1Scene.camera.lookfrom, hw1Scene.camera.lookat, hw1Scene.camera.up, hw1Scene.camera.vfov};
            for (auto hw1Sphere : hw1Scene.shapes)
            {
                shapes.push_back(Sphere{hw1Sphere.center, hw1Sphere.radius, hw1Sphere.material_id});
            }
            for (auto hw1Material : hw1Scene.materials)
            {
                MaterialType type;
                switch (hw1Material.type)
                {
                case hw1::MaterialType::Diffuse:
                    type = MaterialType::Diffuse;
                    break;
                case hw1::MaterialType::Mirror:
                    type = MaterialType::Mirror;
                    break;
                default:
                    break;
                }
                materials.push_back(Material{type, hw1Material.color});
            }
            for (auto hw1Light : hw1Scene.lights)
            {
                lights.push_back(PointLight{hw1Light.intensity, hw1Light.position});
            }
        }
    };

    // Scene parse_scene(const std::vector<std::string> &params);

    void print_scene(const Scene &scene);
}