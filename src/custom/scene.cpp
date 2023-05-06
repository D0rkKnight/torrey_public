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
    scene.materials.push_back(Material(MaterialType::Diffuse, Vector3{1.0, 0.5, 0.5}));

    return scene;
}

Scene::Scene()
{
    camera = AbstractCamera{Vector3{0, 0, 0}, Vector3{0, 0, 1}, Vector3{0, -1, 0}, (Real)90.0};
    shapes = std::vector<Shape *>();
    materials = std::vector<Material>();
    textures = std::map<std::filesystem::path, Image3>();
}

Scene::Scene(ParsedScene parsed)
{
    // Invoke base constructor
    Scene();

    // Copy camera
    camera = AbstractCamera{parsed.camera.lookfrom, parsed.camera.lookat, parsed.camera.up, parsed.camera.vfov};

    // Copy shapes
    for (int i = 0; i < (int)parsed.shapes.size(); i++)
    {
        ParsedShape parsedShape = parsed.shapes[i];
        int matID = get_material_id(parsedShape);

        if (auto sphere = std::get_if<ParsedSphere>(&parsedShape))
        {
            Shape *shape = new Sphere(sphere->position, sphere->radius, matID);
            shapes.push_back(shape);
        }
        else if (auto mesh = std::get_if<ParsedTriangleMesh>(&parsedShape))
        {
            // Build triangle shapes
            for (int i = 0; i < mesh->indices.size(); i++)
            {
                Vector3i index = mesh->indices[i];
                Vector3 v0 = mesh->positions[index[0]];
                Vector3 v1 = mesh->positions[index[1]];
                Vector3 v2 = mesh->positions[index[2]];
                Shape *shape = new Triangle(v0, v1, v2, matID);
                shapes.push_back(shape);
            }
        }
        else
        {
            std::cerr << "Unknown shape type" << std::endl;
            continue;
        }
        // set_area_light_id(shape, parsedShape.area_light_id); Dunno what an area light id is
    }

    // Copy materials
    for (int i = 0; i < (int)parsed.materials.size(); i++)
    {
        ParsedMaterial parsedMaterial = parsed.materials[i];
        Material material;
        material.scene = this;

        if (auto diffuse = std::get_if<ParsedDiffuse>(&parsedMaterial))
        {
            material.type = MaterialType::Diffuse;
            if (auto rgb = std::get_if<Vector3>(&diffuse->reflectance))
            {
                material.flatColor = *rgb;
            }
            else if (auto image_texture = std::get_if<ParsedImageTexture>(&diffuse->reflectance))
            {
                material.loadTexture(image_texture);
            }
        }
        else if (auto mirror = std::get_if<ParsedMirror>(&parsedMaterial))
        {
            material.type = MaterialType::Mirror;
            if (auto rgb = std::get_if<Vector3>(&mirror->reflectance))
            {
                material.flatColor = *rgb;
            }
            else if (auto image_texture = std::get_if<ParsedImageTexture>(&mirror->reflectance))
            {
                material.loadTexture(image_texture);
            }
        }
        else
        {
            std::cerr << "Unknown material type" << std::endl;
            continue;
        }
        materials.push_back(material);
    }

    // Copy lights
    for (int i = 0; i < (int)parsed.lights.size(); i++)
    {
        ParsedLight parsedLight = parsed.lights[i];
        PointLight light;
        if (auto point_light = std::get_if<ParsedPointLight>(&parsedLight))
        {
            light.position = point_light->position;
            light.intensity = point_light->intensity;
        }
        else if (auto diffuse_area_light = std::get_if<ParsedDiffuseAreaLight>(&parsedLight))
        {
            // light.position = shapes[diffuse_area_light->shape_id]->sample_surface();
            // light.intensity = diffuse_area_light->radiance;

            std::cerr << "Diffuse area lights not supported" << std::endl;
        }
        else
        {
            std::cerr << "Unknown light type" << std::endl;
            continue;
        }
        lights.push_back(light);
    }
}

void Scene::addTexture(ParsedImageTexture *image_texture)
{

    // If the texture is not already loaded, load it
    if (textures.find(image_texture->filename) == textures.end())
    {
        Image3 image = imread3(image_texture->filename);
        textures[image_texture->filename] = image;
    }
}

Material::Material()
{
    type = MaterialType::Diffuse;
    flatColor = Vector3{1.0, 1.0, 1.0};
    texMeta = nullptr;
    scene = nullptr;
}

Material::Material(MaterialType type, Vector3 flatColor)
{
    Material();
    this->type = type;
    this->flatColor = flatColor;
}

Vector3 Material::getColor(Real u, Real v)
{

    if (texMeta == nullptr)
        return flatColor;

    // Get the image texture
    Image3 *image = &(scene->textures[texMeta->filename]);

    // Get the pixel coordinates
    int x = (int)(image->width * modulo(texMeta->uscale * u + texMeta->uoffset, 1.0));
    int y = (int)(image->height * modulo(texMeta->vscale * v + texMeta->voffset, 1.0));

    // Get the pixel color
    return (*image)(x, y);
}

void Material::loadTexture(ParsedImageTexture *image_texture)
{

    // Load the image texture and create a texture object
    // ...

    // Clone to heapsince it's getting deallocated later or something
    texMeta = new ParsedImageTexture(*image_texture);

    scene->addTexture(image_texture);
}