#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include "onb.h"
#include <iostream>

using namespace cu_utils;

cu_utils::PhongMaterial::PhongMaterial() : Material(){};

Vector3 cu_utils::PhongMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const
{
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

    if (!scatter(ray, bestHit, albedo, scattered, pdf, rng))
        return emitted;

    // Move scatter forwards a bit to avoid self-intersection
    scattered.origin += scattered.dir * 0.0001;

    Vector3 reflected = reflect(normalize(ray.dir), bestHit.normal);
    Real align = dot(scattered.dir, reflected);
    if (align < 0)
        align = 0;
    
    Real specular = pow(align, exp);
    Real normalizer = (exp + 1) / (2 * MY_PI);

    Vector3 recurse = renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1);

    // TODO: Figure out what this 2PI term is doing here...
    return emitted + 2 * MY_PI *
        albedo * scattering_pdf(ray, bestHit, scattered) * specular * normalizer * recurse / pdf;


    // return emitted
    //      + albedo * scattering_pdf(ray, bestHit, scattered)
    //               * renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1) / pdf;
}


// Borrowed from Peter Shirley's Ray Tracing in One Weekend
bool PhongMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
    onb uvw;
    uvw.build_from_w(hit.normal);
    auto direction = uvw.local(random_cosine_direction(rng));

    scattered = Ray(ray * hit.t, normalize(direction));
    alb = getTexColor(hit.u, hit.v);
    pdf = dot(hit.normal, scattered.dir) / MY_PI;
    return true;
}

double PhongMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto cosine = dot(rec.normal, normalize(scattered.dir));
    return cosine < 0 ? 0 : cosine/MY_PI;
}