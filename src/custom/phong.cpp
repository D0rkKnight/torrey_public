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
    return matte(renderer, ray, bestHit, scene, objRoot, rng, depth);

    // Vector3 color = Vector3{0, 0, 0};
    // Vector3 hit = ray * bestHit.t;

    // // Perform diffuse interreflection now
    // if (depth <= 0)
    //     return Vector3{0, 0, 0};
    
    // // Copied from Peter Shirley's Ray Tracing in One Weekend
    // Ray scattered;
    // Vector3 attenuation;
    // Vector3 emitted = bestHit.sphere->areaLight ? bestHit.sphere->areaLight->intensity : Vector3{0, 0, 0};
    // Real pdf;
    // Vector3 albedo;

    // // Make sure we hit on the right side, otherwise emitted light is 0
    // if (bestHit.backface)
    //     emitted = Vector3{0, 0, 0};

    // if (!scatter(ray, bestHit, albedo, scattered, pdf, rng))
    //     return emitted;

    // // Move scatter forwards a bit to avoid self-intersection
    // scattered.origin += scattered.dir * 0.0001;

    // Vector3 reflected = reflect(normalize(ray.dir), bestHit.normal);
    // Real align = dot(scattered.dir, reflected);
    // if (align < 0)
    //     align = 0;
    
    // Real specular = pow(align, exp);
    // Real normalizer = (exp + 1) / (2 * MY_PI);

    // Vector3 recurse = renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1);

    // // TODO: Figure out what this 2PI term is doing here...
    // return emitted + 2 * MY_PI *
    //     albedo * scattering_pdf(ray, bestHit, scattered) * specular * normalizer * recurse / pdf;


    // return emitted
    //      + albedo * scattering_pdf(ray, bestHit, scattered)
    //               * renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1) / pdf;
}


// Borrowed from Peter Shirley's Ray Tracing in One Weekend
bool PhongMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
    // onb uvw;
    // uvw.build_from_w(hit.normal);
    // auto direction = uvw.local(random_cosine_direction(rng));

    // scattered = Ray(ray * hit.t, normalize(direction));
    // alb = getTexColor(hit.u, hit.v);
    // pdf = dot(hit.normal, scattered.dir) / MY_PI;

    Real u1 = next_pcg32_real<Real>(rng);
    Real u2 = next_pcg32_real<Real>(rng);

    Real cosTheta = pow(u1, 1 / (exp + 1));
    Real phi = 2 * MY_PI * u2;

    Vector3 reflected = reflect(normalize(ray.dir), hit.normal);
    onb uvw = onb();
    uvw.build_from_w(reflected);

    // Get ray in aroundR space from cosTheta and phi
    Real z = cosTheta;
    Real sinTheta = sqrt(1 - cosTheta * cosTheta);
    Real x = cos(phi) * sinTheta;
    Real y = sin(phi) * sinTheta;

    Vector3 direction = uvw.local(Vector3{x, y, z});
    scattered = Ray(ray * hit.t, direction);
    alb = getTexColor(hit.u, hit.v);

    // Real sinTheta = sqrt(1 - cosTheta * cosTheta);

    pdf = (exp + 1) * pow(dot(direction, reflected), exp) / (2 * MY_PI);
    return true;
}

double PhongMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto reflectRay = Ray(r_in * rec.t, normalize(reflect(normalize(r_in.dir), rec.normal)));
    auto cosine = dot(reflectRay.dir, scattered.dir);
    return cosine < 0 ? 0 : pow(cosine, exp) / (2 * MY_PI) * (exp + 1);
}