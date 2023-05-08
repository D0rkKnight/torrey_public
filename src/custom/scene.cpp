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
    Material *lambert = new LambertMaterial();
    lambert->flatColor = Vector3{1.0, 0.5, 0.5};
    scene.materials.push_back(lambert);

    return scene;
}

Scene::Scene()
{
    camera = AbstractCamera{Vector3{0, 0, 0}, Vector3{0, 0, 1}, Vector3{0, -1, 0}, (Real)90.0};
    shapes = std::vector<Shape *>();
    materials = std::vector<Material *>();
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
                Triangle *tri = new Triangle(v0, v1, v2, matID);

                // Pick up UVs too
                if (mesh->uvs.size() > 0)
                {
                    Vector2 uv0 = mesh->uvs[index[0]];
                    Vector2 uv1 = mesh->uvs[index[1]];
                    Vector2 uv2 = mesh->uvs[index[2]];
                    tri->setUVs(uv0, uv1, uv2);
                }

                // I think normal splitting is already done for us
                if (mesh->normals.size() > 0)
                {
                    Vector3 n0 = mesh->normals[index[0]];
                    Vector3 n1 = mesh->normals[index[1]];
                    Vector3 n2 = mesh->normals[index[2]];
                    tri->n0 = n0;
                    tri->n1 = n1;
                    tri->n2 = n2;
                }

                shapes.push_back(tri);
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
        Material *material;
        ParsedColor colSrc;

        if (auto diffuse = std::get_if<ParsedDiffuse>(&parsedMaterial))
        {
            material = new LambertMaterial();
            colSrc = diffuse->reflectance;
        }
        else if (auto mirror = std::get_if<ParsedMirror>(&parsedMaterial))
        {
            material = new MirrorMaterial();
            colSrc = mirror->reflectance;
        }
        else if (auto plastic = std::get_if<ParsedPlastic>(&parsedMaterial))
        {
            material = new PlasticMaterial();
            colSrc = plastic->reflectance;
            material->eta = plastic->eta;
        }
        else
        {
            std::cerr << "Unknown material type" << std::endl;
            continue;
        }
        material->scene = this;
        assignParsedColor(material, colSrc);
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

// TODO: Move this to the material src probably
Vector3 Material::getTexColor(Real u, Real v)
{

    if (texMeta == nullptr)
        return flatColor;

    // Get the image texture
    Image3 *image = &(scene->textures[texMeta->filename]);

    // Get the pixel coordinates
    Real rx = (image->width * modulo(texMeta->uscale * u + texMeta->uoffset, 1.0));
    Real ry = (image->height * modulo(texMeta->vscale * v + texMeta->voffset, 1.0));

    // Bilinear interpolation
    int x = (int)rx;
    int y = (int)ry;
    int nx = (x + 1) % image->width;
    int ny = (y + 1) % image->height;

    // Get the four surrounding pixels
    Vector3 c00 = (*image)(x, y);
    Vector3 c01 = (*image)(x, ny);
    Vector3 c10 = (*image)(nx, y);
    Vector3 c11 = (*image)(nx, ny);

    // Interpolate
    Real dx = rx - x;
    Real dy = ry - y;
    Vector3 c0 = c00 * (1 - dx) + c10 * dx;
    Vector3 c1 = c01 * (1 - dx) + c11 * dx;
    Vector3 c = c0 * (1 - dy) + c1 * dy;

    // Get the pixel color
    return c;
}

void Material::loadTexture(ParsedImageTexture *image_texture)
{

    // Load the image texture and create a texture object
    // ...

    // Clone to heapsince it's getting deallocated later or something
    texMeta = new ParsedImageTexture(*image_texture);

    scene->addTexture(image_texture);
}

void cu_utils::assignParsedColor(Material *material, ParsedColor color)
{
    if (auto rgb = std::get_if<Vector3>(&color))
    {
        material->flatColor = *rgb;
    }
    else if (auto image_texture = std::get_if<ParsedImageTexture>(&color))
    {
        material->loadTexture(image_texture);
    }
}