#pragma once

#include "utils.h"
#include <iostream>
#include <vector>
#include <map>
#include "shapes.h"
#include "../parse_scene.h"
#include "materials.h"

namespace cu_utils
{
    class Scene;

    struct PointLight
    {
        Vector3 intensity;
        Vector3 position;
    };

    struct AreaLight
    {
        Vector3 intensity;
        std::vector<Shape *> shapes;

        AreaLight(Vector3 intensity);
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
        std::vector<Shape *> shapes;
        std::vector<Material *> materials;
        std::vector<PointLight> lights;
        std::vector<AreaLight *> areaLights;

        std::map<std::filesystem::path, Image3> textures;

        Scene();
        Scene(const ParsedScene &parsedScene);

        static Scene defaultScene();

        void addTexture(ParsedImageTexture *texMeta);
    };

    void assignParsedColor(Material *material, ParsedColor color);
}