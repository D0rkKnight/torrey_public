#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include "onb.h"
#include <iostream>

using namespace cu_utils;

cu_utils::BlinnPhongMaterial::BlinnPhongMaterial() : Material(){};

Vector3 cu_utils::BlinnPhongMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const
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

    // Move scatter forwards a bit to avoid self-intersection
    scattered.origin += scattered.dir * 0.0001;

    // Get fresnel as well (this makes it different from matte)
    auto half = normalize(scattered.dir + -ray.dir);
    Vector3 fresnel = fresnelSchlick(albedo, scattered.dir, half);

    Real coeff = (exp+2) / (4 * MY_PI * (2-pow(2, -exp/2)));

    return emitted
         + fresnel * coeff * pow(dot(bestHit.normal, half), exp)
        * renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1) / pdf;
}


// TODO: Implement Blinn Phong sampling rather than just using Phong sampling
bool BlinnPhongMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
    Real u1 = next_pcg32_real<Real>(rng);
    Real u2 = next_pcg32_real<Real>(rng);

    Real cosTheta = pow(u1, 1 / (exp + 1));
    Real phi = 2 * MY_PI * u2;

    Real sinTheta = sqrt(1 - cosTheta * cosTheta);
    Vector3 half = Vector3{cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta};

    onb uvw = onb();
    uvw.build_from_w(hit.normal);
    half = uvw.local(half); // Orthogonal so outcome will be normal

    Vector3 direction = reflect(ray.dir, half);
    scattered = Ray(ray * hit.t, direction);
    alb = getTexColor(hit.u, hit.v);

    pdf = scattering_pdf(ray, hit, scattered);

    return true;

}

double BlinnPhongMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto half = normalize(-r_in.dir + scattered.dir);
    Real cosine = dot(rec.normal, half);
    Real pdf = (exp + 1) * pow(cosine, exp) / (2 * MY_PI * 4 * dot(scattered.dir, half));

    return pdf;
}