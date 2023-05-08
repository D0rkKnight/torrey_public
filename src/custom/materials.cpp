#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include <iostream>

using namespace cu_utils;

Vector3 cu_utils::lambert(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot)
{
    Material *material = scene.materials[bestHit.sphere->material_id];

    Vector3 color = Vector3{0, 0, 0};
    Vector3 hit = ray * bestHit.t;

    for (const PointLight &light : scene.lights)
    {
        // Use 0 0 uv since we don't have the uv coord from the ray hit yet
        Vector3 albedo = material->getTexColor(bestHit.u, bestHit.v);
        Vector3 lightDir = normalize(light.position - hit);

        Real diffuse = std::max(dot(lightDir, bestHit.normal), 0.0);
        Vector3 contribution = albedo * diffuse;

        contribution /= c_PI;

        Real distSqrd = distance_squared(light.position, hit);
        Vector3 attenuation = light.intensity / distSqrd;

        contribution = Vector3{contribution.x * attenuation.x, contribution.y * attenuation.y, contribution.z * attenuation.z};

        // Shadow cast
        Ray shadowRay = Ray(hit, lightDir);
        // Move the ray forward by 10^-4
        shadowRay.origin += shadowRay.dir * 0.0001;

        RayHit shadowHit = renderer->castRay(shadowRay, scene.shapes, objRoot);
        if (shadowHit.hit && shadowHit.t < distance(light.position, hit))
        {
            // If the shadow ray hit something, then it's in shadow
            contribution *= 0.0;
        }

        color += contribution;
    }

    return color;
}

Vector3 cu_utils::mirror(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth)
{
    Material *material = scene.materials[bestHit.sphere->material_id];

    Ray reflectRay = getBounceRay(ray, bestHit);

    // Recurse
    Vector3 albedo = material->getTexColor(bestHit.u, bestHit.v);
    Vector3 fresnel = fresnelSchlick(albedo, bestHit.normal, reflectRay.dir);

    return hadamard(fresnel, renderer->getPixelColor(reflectRay, scene, objRoot, depth - 1));
}

Vector3 cu_utils::plastic(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth)
{
    Material *material = scene.materials[bestHit.sphere->material_id];

    // Compute reflect component
    Ray reflectRay = getBounceRay(ray, bestHit);
    Vector3 albedo = material->getTexColor(bestHit.u, bestHit.v);

    // Recurse
    Vector3 fresnel = fresnelSchlick(albedo, bestHit.normal, reflectRay.dir);
    Vector3 reflectColor = renderer->getPixelColor(reflectRay, scene, objRoot, depth - 1);

    // Get the diffuse color from the mat
    // Lambert retrieves the albedo independently
    Vector3 diffuseCol = lambert(renderer, ray, bestHit, scene, objRoot);

    // Get the diffuse contribution
    Vector3 diffuse = diffuseCol * (1.0 - fresnel);

    // Get the specular contribution
    Vector3 specular = hadamard(fresnel, reflectColor);

    return diffuse + specular;
}

// Write in material methods
Vector3 cu_utils::Material::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth) const
{
    std::cerr << "Material::shadePoint() called, this should never happen\n"
              << std::endl;
    return Vector3{0, 0, 0};
}

Vector3 cu_utils::LambertMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth = 0) const
{
    return lambert(renderer, ray, bestHit, scene, objRoot);
}

Vector3 cu_utils::MirrorMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth = 0) const
{
    if (depth <= 0)
        return lambert(renderer, ray, bestHit, scene, objRoot);

    return mirror(renderer, ray, bestHit, scene, objRoot, depth);
}

Vector3 cu_utils::PlasticMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth = 0) const
{
    return plastic(renderer, ray, bestHit, scene, objRoot, depth);
}

// Constructors
cu_utils::Material::Material()
{
    this->flatColor = Vector3{1, 1, 1};
    this->scene = nullptr;
    this->eta = 0;
    this->texMeta = nullptr;
}

cu_utils::LambertMaterial::LambertMaterial() : Material(){};
cu_utils::MirrorMaterial::MirrorMaterial() : Material(){};
cu_utils::PlasticMaterial::PlasticMaterial() : Material(){};