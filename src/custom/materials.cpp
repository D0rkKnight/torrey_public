#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include "onb.h"
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
    Ray reflectRay = getBounceRay(ray, bestHit);

    // Recurse
    Real fresnelN = 1.5;
    Real Fz = (fresnelN - 1) * (fresnelN - 1) / ((fresnelN + 1) * (fresnelN + 1));
    Vector3 fresnel = fresnelSchlick(Vector3{Fz, Fz, Fz}, bestHit.normal, reflectRay.dir);

    // Use unbiased estimation for diffuse vs specular
    // Get average of fresnel for weighting (just fresnel.x since we use uniform color for F0)
    Real avgFresnel = (fresnel.x + fresnel.y + fresnel.z) / 3;
    if (next_pcg32_real<Real>(rng) > avgFresnel)
        return matte(renderer, ray, bestHit, scene, objRoot, rng, depth);

    reflectRay.origin = reflectRay.origin + reflectRay.dir * 0.001;
    Vector3 reflectColor = renderer->getPixelColor(reflectRay, scene, objRoot, rng, depth - 1);
    // Vector3 specular = hadamard(fresnel, reflectColor);

    return reflectColor;
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

void PlasticMaterial::finish()
{
    this->backingLambert = LambertMaterial();
    this->backingLambert.flatColor = this->flatColor;
    this->backingLambert.scene = this->scene;
    this->backingLambert.eta = this->eta;
    this->backingLambert.texMeta = this->texMeta;
}

bool PlasticMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
    onb uvw;
    uvw.build_from_w(hit.normal);
    auto direction = uvw.local(random_cosine_direction(rng));

    scattered = Ray(ray * hit.t, normalize(direction));
    alb = getTexColor(hit.u, hit.v);
    pdf = scattering_pdf(ray, hit, scattered);
    return true;
}

double PlasticMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto cosine = dot(rec.normal, normalize(scattered.dir));
    return cosine < 0 ? 0 : cosine/MY_PI;
}

Real PlasticMaterial::light_contribution(const Ray &ray_in, const RayHit &hit, const Ray &ray_out) const {
    // Turns out the sampling distribution is already perfectly matched to the pdf
    return scattering_pdf(ray_in, hit, ray_out);
}