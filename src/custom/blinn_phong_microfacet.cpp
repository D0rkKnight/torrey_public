#include "materials.h"
#include "scene.h"
#include "shapes.h"
#include "utils.h"
#include "ray.h"
#include "renderer.h"
#include "onb.h"
#include <iostream>

using namespace cu_utils;

cu_utils::MicrofacetMaterial::MicrofacetMaterial() : Material(){};

Real genGeomShadowMask(const RayHit &bestHit, const Vector3 &dir, const Real exp) {
    Real cosTheta = dot(bestHit.normal, dir);
    Real tanTheta = sqrt(1 - cosTheta * cosTheta) / cosTheta;
    Real a = sqrt(0.5 * exp + 1)/tanTheta;
    Real geomShadowMask = 0;

    if (dot(dir, bestHit.normal) > 0) {
        geomShadowMask = 1;

        if (a < 1.6) {
            geomShadowMask = 3.535 * a + 2.181 * a * a;
            geomShadowMask /= 1 + 2.276 * a + 2.577 * a * a;
        }
    }

    return geomShadowMask;
}

// Should prolly pass by reference here
Vector3 cu_utils::MicrofacetMaterial::shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const
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
    Real ndf = (exp+2)/(2*MY_PI) * pow(dot(bestHit.normal, half), exp);

    Real geomShadowMask = genGeomShadowMask(bestHit, scattered.dir, exp) *
                        genGeomShadowMask(bestHit, -ray.dir, exp);

    // Honestly don't think this ever happens but just in case
    if (dot(scattered.dir, bestHit.normal) <= 0)
        return Vector3{0, 0, 0};

    Vector3 coeff = fresnel * ndf * geomShadowMask / (4 * dot(bestHit.normal, -ray.dir)) / pdf;

    // if (coeff.x > 1 || coeff.y > 1 || coeff.z > 1)
    //     std::cout << "Coeff greater than 1: " << coeff << std::endl;

    return emitted
         + coeff * renderer->getPixelColor(scattered, scene, objRoot, rng, depth - 1);
}

/**
 * Just use Blinn Phong scattering
*/
bool MicrofacetMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
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

double MicrofacetMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto half = normalize(-r_in.dir + scattered.dir);
    Real cosine = dot(rec.normal, half);
    Real pdf = (exp + 1) * pow(cosine, exp) / (2 * MY_PI * 4 * dot(scattered.dir, half));

    return pdf;
}