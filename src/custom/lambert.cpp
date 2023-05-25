#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include <iostream>

using namespace cu_utils;

cu_utils::LambertMaterial::LambertMaterial() : Material(){};

Vector3 cu_utils::matte(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth)
{
    Material *material = scene.materials[bestHit.sphere->material_id];

    // Check if material is a plastic and do this weird forced fallback thing
    if (PlasticMaterial *plastic = dynamic_cast<PlasticMaterial *>(material))
        material = &plastic->backingLambert;

    Vector3 color = Vector3{0, 0, 0};
    Vector3 hit = ray * bestHit.t;

    // Perform diffuse interreflection now
    if (depth <= 0)
        return Vector3{0, 0, 0};
    
    // Copied from Peter Shirley's Ray Tracing in One Weekend
    Ray scattered;
    Vector3 attenuation;
    Vector3 emitted = bestHit.sphere->areaLight ? bestHit.sphere->areaLight->intensity : Vector3{0, 0, 0};
    Real pdf;
    Vector3 albedo;

    // Make sure we hit on the right side, otherwise emitted light is 0
    if (bestHit.backface)
        emitted = Vector3{0, 0, 0};

    if (!material->scatter(ray, bestHit, albedo, scattered, pdf, rng))
        return emitted;

    // Move scatter forwards a bit to avoid self-intersection
    scattered.origin += scattered.dir * 0.0001;

    return emitted
         + albedo * material->scattering_pdf(ray, bestHit, scattered)
                  * renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1) / pdf;
}
Vector3 cu_utils::LambertMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const
{
    return matte(renderer, ray, bestHit, scene, objRoot, rng, depth);
}


// Borrowed from Peter Shirley's Ray Tracing in One Weekend
bool LambertMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
    Vector3 scatter_direction = hit.normal + randomUnitVector(rng);
    Vector3 hitp = ray * hit.t;

    // Catch degenerate scatter direction
    if (length_squared(scatter_direction) < 0.0001)
        scatter_direction = hit.normal;
    scattered = Ray(ray * hit.t, normalize(scatter_direction));
    alb = getTexColor(hit.u, hit.v);
    pdf = dot(hit.normal, scattered.dir) / MY_PI;
    return true;
}

double LambertMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto cosine = dot(rec.normal, normalize(scattered.dir));
    return cosine < 0 ? 0 : cosine/MY_PI;
}