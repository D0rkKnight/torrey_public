#pragma once

#include "utils.h"
#include <iostream>
#include <vector>
#include <map>
#include "shapes.h"
#include "../parse_scene.h"

namespace cu_utils
{
    class Scene;

    enum class MaterialType
    {
        Diffuse,
        Mirror
    };

    struct Material
    {
        MaterialType type;
        Vector3 flatColor;
        Scene *scene;

        // Can have a backing image texture.
        ParsedImageTexture *texMeta;

        Material();
        Material(MaterialType type, Vector3 flatColor);

        Vector3 getColor(Real u, Real v);
        void loadTexture(ParsedImageTexture *texMeta);
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
        std::vector<Shape *> shapes;
        std::vector<Material> materials;
        std::vector<PointLight> lights;

        std::map<std::filesystem::path, Image3> textures;

        Scene();
        Scene(ParsedScene parsedScene);

        static Scene defaultScene();

        void addTexture(ParsedImageTexture *texMeta);
    };
}