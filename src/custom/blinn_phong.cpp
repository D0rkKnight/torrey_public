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
    return matte(renderer, ray, bestHit, scene, objRoot, rng, depth);
}


bool BlinnPhongMaterial::scatter(const Ray &ray, const RayHit &hit, Vector3 &alb, Ray &scattered, Real &pdf, pcg32_state &rng) const {
    Real u1 = next_pcg32_real<Real>(rng);
    Real u2 = next_pcg32_real<Real>(rng);

    // Randomly sample half vector for basis ns
    Real cosTheta = pow(u1, 1 / (exp + 1));

    // Real cosTheta = pow(u1, 1 / (exp + 1));
    // Real phi = 2 * MY_PI * u2;

    // Vector3 reflected = reflect(normalize(ray.dir), hit.normal);
    // onb uvw = onb();
    // uvw.build_from_w(reflected);

    // // Get ray in aroundR space from cosTheta and phi
    // Real z = cosTheta;
    // Real sinTheta = sqrt(1 - cosTheta * cosTheta);
    // Real x = cos(phi) * sinTheta;
    // Real y = sin(phi) * sinTheta;

    // Vector3 direction = uvw.local(Vector3{x, y, z});
    // scattered = Ray(ray * hit.t, direction);
    // alb = getTexColor(hit.u, hit.v);

    // pdf = (exp + 1) * pow(dot(direction, reflected), exp) / (2 * MY_PI);
    // return true;

}

double BlinnPhongMaterial::scattering_pdf(
    const Ray& r_in, const RayHit& rec, const Ray& scattered
) const {
    auto half = normalize(-r_in.dir + scattered.dir);
    auto cosine = dot(half, rec.normal);

    return cosine < 0 ? 0 : pow(cosine, exp) * (exp + 1) / (2 * MY_PI * 4 * dot(scattered.dir, half)) ;

    // auto reflectRay = Ray(r_in * rec.t, normalize(reflect(normalize(r_in.dir), rec.normal)));
    // auto cosine = dot(reflectRay.dir, scattered.dir);
    // return cosine < 0 ? 0 : pow(cosine, exp) / (2 * MY_PI) * (exp + 1);
}