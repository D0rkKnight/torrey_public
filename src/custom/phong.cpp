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
    // JANK incoming: do pdf mixing here
    // Will require scattering pdf to be implemented to generate the probability of a scattered ray to be from the BRDF pdf.
    // Also needs area light sampling functionality
    // And scattering pdf for area lights

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

    // Pick one area light at random and sample it
    int lightIndex, shapeIndex;
    Shape *emitter;
    int numAreaLights = scene.areaLights.size();
    
    if (numAreaLights > 0) {
        lightIndex = next_pcg32_real<Real>(rng) * scene.areaLights.size();

        // Pick a shape on the area light
        shapeIndex = next_pcg32_real<Real>(rng) * scene.areaLights[lightIndex]->shapes.size();
        emitter = scene.areaLights[lightIndex]->shapes[shapeIndex];
    }

    Real scatterOrLight = next_pcg32_real<Real>(rng);
    if (scatterOrLight <= 0.5 || numAreaLights == 0) {
        if (!scatter(ray, bestHit, albedo, scattered, pdf, rng))
            return emitted;
    } else {
        // Sample the shape
        Real jacobian;
        Ray emissionRay = emitter->sampleSurface(1, jacobian, rng);
        Vector3 scatterDir = normalize(emissionRay.origin - hit);

        scattered = Ray(hit, scatterDir);
        albedo = getTexColor(bestHit.u, bestHit.v);
    }

    // What is pdf? It is the joint probability of both the sampler and the emitter! :D
    if (numAreaLights > 0)
        pdf = 0.5 * scattering_pdf(ray, bestHit, scattered) + 0.5 * emitter->pdfSurface(scattered);
    else
        pdf = scattering_pdf(ray, bestHit, scattered);

    if (dot(scattered.dir, bestHit.normal) < 0)
        return emitted;

    // Move scatter forwards a bit to avoid self-intersection
    scattered.origin += scattered.dir * 0.0001;
    return emitted
         + albedo * scattering_pdf(ray, bestHit, scattered)
                  * renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1) / pdf;
}


bool PhongMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
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

    pdf = scattering_pdf(ray, hit, scattered);
    return true;
}

double PhongMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    Vector3 reflected = reflect(normalize(r_in.dir), rec.normal);
    return (exp + 1) * pow(dot(scattered.dir, reflected), exp) / (2 * MY_PI);
}