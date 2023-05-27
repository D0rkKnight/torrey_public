#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include "onb.h"
#include <iostream>

using namespace cu_utils;

cu_utils::LambertMaterial::LambertMaterial() : Material(){};

Vector3 cu_utils::matte(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth)
{
    Material *material = scene.materials[bestHit.sphere->material_id];

    // Check if material is a plastic and do this weird forced fallback thing
    // if (PlasticMaterial *plastic = dynamic_cast<PlasticMaterial *>(material))
    //     material = &plastic->backingLambert;

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
    numAreaLights = 0;

    if (numAreaLights > 0) {
        lightIndex = next_pcg32_real<Real>(rng) * scene.areaLights.size();

        // Pick a shape on the area light
        shapeIndex = next_pcg32_real<Real>(rng) * scene.areaLights[lightIndex]->shapes.size();
        emitter = scene.areaLights[lightIndex]->shapes[shapeIndex];
    }

    Real scatterOrLight = next_pcg32_real<Real>(rng);
    if (scatterOrLight <= 0.5 || numAreaLights == 0) {
        if (!material->scatter(ray, bestHit, albedo, scattered, pdf, rng))
            return emitted;
    } else {
        // Sample the shape
        Real jacobian;
        Ray emissionRay = emitter->sampleSurface(1, jacobian, rng);
        Vector3 scatterDir = normalize(emissionRay.origin - hit);

        scattered = Ray(hit, scatterDir);
        albedo = material->getTexColor(bestHit.u, bestHit.v);
    }

    // material->scatter(ray, bestHit, albedo, scattered, pdf, rng);
    

    // What is pdf? It is the joint probability of both the sampler and the emitter! :D
    if (numAreaLights > 0)
        pdf = 0.5 * material->scattering_pdf(ray, bestHit, scattered) + 0.5 * emitter->pdfSurface(scattered);
    else
        pdf = material->scattering_pdf(ray, bestHit, scattered);

    // Move scatter forwards a bit to avoid self-intersection
    scattered.origin += scattered.dir * 0.0001;
    return emitted
         + albedo * material->light_contribution(ray, bestHit, scattered)
                  * renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1) / pdf;
}

Vector3 cu_utils::LambertMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const
{
    return matte(renderer, ray, bestHit, scene, objRoot, rng, depth);
}


// Borrowed from Peter Shirley's Ray Tracing in One Weekend
bool LambertMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
    onb uvw;
    uvw.build_from_w(hit.normal);
    auto direction = uvw.local(random_cosine_direction(rng));

    scattered = Ray(ray * hit.t, normalize(direction));
    alb = getTexColor(hit.u, hit.v);
    pdf = scattering_pdf(ray, hit, scattered);
    return true;
}

double LambertMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto cosine = dot(rec.normal, normalize(scattered.dir));
    return cosine < 0 ? 0 : cosine/MY_PI;
}

Real LambertMaterial::light_contribution(const Ray &ray_in, const RayHit &hit, const Ray &ray_out) const {
    // Turns out the sampling distribution is already perfectly matched to the pdf
    return scattering_pdf(ray_in, hit, ray_out);
}
