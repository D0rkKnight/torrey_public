#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include <iostream>

using namespace cu_utils;

Vector3 cu_utils::mirror(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth)
{
    Material *material = scene.materials[bestHit.sphere->material_id];

    Ray reflectRay = getBounceRay(ray, bestHit);

    // Recurse
    Vector3 albedo = material->getTexColor(bestHit.u, bestHit.v);
    Vector3 fresnel = fresnelSchlick(albedo, bestHit.normal, reflectRay.dir);

    return hadamard(fresnel, renderer->getPixelColor(reflectRay, scene, objRoot, rng, depth - 1));
}

Vector3 cu_utils::plastic(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth)
{
    Material *material = scene.materials[bestHit.sphere->material_id];

    // Compute reflect component
    Vector3 albedo = material->getTexColor(bestHit.u, bestHit.v);
    Ray reflectRay = getBounceRay(ray, bestHit);

    // Recurse
    Vector3 fresnel = fresnelSchlick(albedo, bestHit.normal, reflectRay.dir);

    // Use unbiased estimation for diffuse vs specular
    if (next_pcg32_real<Real>(rng) > 1 - length(fresnel))
        return matte(renderer, ray, bestHit, scene, objRoot, rng, depth);


    Vector3 reflectColor = renderer->getPixelColor(reflectRay, scene, objRoot, rng, depth - 1);
    Vector3 specular = hadamard(fresnel, reflectColor);

    return specular;
}

// Write in material methods
Vector3 cu_utils::Material::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const
{
    std::cerr << "Material::shadePoint() called, this should never happen\n"
              << std::endl;
    return Vector3{0, 0, 0};
}

Vector3 cu_utils::MirrorMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth = 0) const
{
    if (depth <= 0)
        return matte(renderer, ray, bestHit, scene, objRoot, rng, depth);

    return mirror(renderer, ray, bestHit, scene, objRoot, rng, depth);
}

Vector3 cu_utils::PlasticMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth = 0) const
{
    return plastic(renderer, ray, bestHit, scene, objRoot, rng, depth);
}

// Constructors
cu_utils::Material::Material()
{
    this->flatColor = Vector3{1, 1, 1};
    this->scene = nullptr;
    this->eta = 0;
    this->texMeta = nullptr;
}

cu_utils::MirrorMaterial::MirrorMaterial() : Material(){};
cu_utils::PlasticMaterial::PlasticMaterial() : Material(){};