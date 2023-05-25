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