#include "shapes.h"
#include "utils.h"
#include "pcg.h"
#include <iostream>
#include <cmath>
#include <vector.h>

using namespace cu_utils;
// using namespace std;

RayHit Sphere::checkHit(const Ray &ray, const Real mint, const Real maxt) const
{
    // Borrowed from RT in One Weekend
    Vector3 oc = ray.origin - center; // Vector from origin to center of sphere
    auto a = dot(ray.dir, ray.dir);
    auto b = 2 * dot(oc, ray.dir);
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;

    if (discriminant > 0)
    {
        // Return distance to hit point

        // Check the closer hit point first
        Real t = (-b - sqrt(discriminant)) / (2.0 * a);
        Vector3 n = normalize(ray * t - center);

        // uv should be based on normal (before any inner-face inversion)
        auto theta = acos(n.y);
        auto phi = atan2(-n.z, n.x) + MY_PI;

        Real u = phi / (2 * MY_PI);
        Real v = theta / MY_PI;

        bool backface = false;
        if (t <= mint || t >= maxt)
        {
            // Check the further hit point
            t = (-b + sqrt(discriminant)) / (2.0 * a);

            // Both potential hit points are out of bounds, invalid.
            if (t <= mint || t >= maxt)
                return RayHit();

            // normal is pointing the other way in this case
            n = -normalize(ray * t - center);
            backface = true;
        }

        // Valid rayhit found, returning
        RayHit hit = RayHit(true, t, this, n, u, v, backface);

        return hit;
    }
    else
    {
        return RayHit(); // Missed
    }
};

Sphere::Sphere(Vector3 center, Real radius, int material_id)
{
    this->center = center;
    this->radius = radius;
    this->material_id = material_id;
    this->areaLight = nullptr;
}

BoundingBox Sphere::getBoundingBox() const
{
    return BoundingBox(center - Vector3(1, 1, 1) * radius, center + Vector3(1, 1, 1) * radius);
}

Ray Sphere::sampleSurface(int samples, Real &jacobian, pcg32_state &rng) const
{
    // Sample points on the unit sphere
    Real u1 = next_pcg32_real<Real>(rng);
    Real u2 = next_pcg32_real<Real>(rng);

    Real theta = acos(1 - 2 * u1);
    Real phi = 2 * MY_PI * u2;

    Real x = sin(theta) * cos(phi);
    Real y = sin(theta) * sin(phi);
    Real z = cos(theta);

    Vector3 dir = Vector3(x, y, z);

    // Compute jacobian
    jacobian = 4 * MY_PI * this->radius * this->radius;

    // Create ray
    Ray ray = Ray(center + dir * radius, dir);
    
    return ray;
}

Triangle::Triangle(Vector3 v0, Vector3 v1, Vector3 v2, int material_id)
{
    this->v0 = v0;
    this->v1 = v1;
    this->v2 = v2;
    this->material_id = material_id;
    this->areaLight = nullptr;

    // Set uvs so the uv of a hit is just the barycentric coordinates
    uv0 = Vector2(0, 0);
    uv1 = Vector2(1, 0);
    uv2 = Vector2(0, 1);

    // Generate front facing normals from vertices
    n0 = normalize(cross(v1 - v0, v2 - v0));
    n1 = n0;
    n2 = n0;
}

RayHit Triangle::checkHit(const Ray &ray, const Real mint, const Real maxt) const
{
    // Borrowed from RT in One Weekend
    Vector3 e1 = v1 - v0; // v0 -> v1
    Vector3 e2 = v2 - v0; // v0 -> v2
    Vector3 h = cross(ray.dir, e2);
    Real a = dot(e1, h);

    // This code was causing bugs for some reason idk why
    // if (a > -0.00001 && a < 0.00001)
    //     return RayHit(); // Ray is parallel to triangle

    Real f = 1 / a;
    Vector3 s = ray.origin - v0;
    Real u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return RayHit(); // Ray misses triangle

    Vector3 q = cross(s, e1);
    Real v = f * dot(ray.dir, q);

    if (v < 0.0 || u + v > 1.0)
        return RayHit(); // Ray misses triangle

    // At this point, the ray hits the triangle
    Real t = f * dot(e2, q);

    if (t < mint || t > maxt)
        return RayHit(); // Triangle is out of bounds for the raycast

    // n is the weighted average of the triangle's normals
    Vector3 n = normalize(n0 + u * (n1 - n0) + v * (n2 - n0));

    bool backface = dot(n, ray.dir) > 0;
    n = backface ? -n : n;

    // Use barycentric
    Vector3 bary = getBarycentric(ray * t);
    Vector2 uv = bary.x * uv0 + bary.y * uv1 + bary.z * uv2;

    return RayHit(true, t, this, n, uv.x, uv.y, backface);
}

Vector3 Triangle::getBarycentric(const Vector3 p) const
{
    // Compute vectors
    Vector3 b2v1 = v1 - v0;
    Vector3 b2v2 = v2 - v0;
    Vector3 b2p = p - v0;

    // Compute dot products
    Real dot00 = dot(b2v1, b2v1);
    Real dot01 = dot(b2v1, b2v2);
    Real dot02 = dot(b2v1, b2p);
    Real dot11 = dot(b2v2, b2v2);
    Real dot12 = dot(b2v2, b2p);

    // Compute barycentric coordinates
    Real invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    Real u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    Real v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return Vector3(1 - u - v, u, v);
}

BoundingBox Triangle::getBoundingBox() const
{
    Vector3 minv = min<Real>(v0, min<Real>(v1, v2));
    Vector3 maxv = max<Real>(v0, max<Real>(v1, v2));
    return BoundingBox(minv, maxv);
}

Ray Triangle::sampleSurface(int samples, Real &jacobian, pcg32_state &rng) const
{
    // Sample barycentric coordinates
    Real u1 = next_pcg32_real<Real>(rng);
    Real u2 = next_pcg32_real<Real>(rng);

    Real b1 = 1 - sqrt(u1);
    Real b2 = sqrt(u1) * u2;

    // Compute the point on the triangle
    Vector3 p = b2 * v2 + b1 * v1 + (1 - b1 - b2) * v0;

    // Get geometric normal as well
    Vector3 n = normalize(cross(v1 - v0, v2 - v0));


    // Area of the triangle
    jacobian = length<Real>(cross(v1 - v0, v2 - v0)) / 2;

    return Ray(p, n);
}

void Triangle::setUVs(Vector2 uv0, Vector2 uv1, Vector2 uv2)
{
    this->uv0 = uv0;
    this->uv1 = uv1;
    this->uv2 = uv2;
}

Ray Shape::sampleSurface(int samples, Real &jacobian, pcg32_state &rng) const
{
    std::cerr << "Shape::sampleSurface is illegal to call" << std::endl;

    return Ray(Vector3(), Vector3());
}

Real Sphere::pdfSurface(const Ray &ray) const
{
    RayHit hit = checkHit(ray, 0, INFINITY);
    if (!hit.hit)
        return 0;

    // Compute the area of the sphere
    Real area = 4 * MY_PI * radius * radius;

    // Compute the pdf
    Real pdf = hit.t * hit.t / (dot(-ray.dir, hit.normal) * area);

    // return pdf;
    // Doubling factor because each ray could have been sampled from the front or back
    // So the pdf density is actually double for rays
    return pdf * 2;
}

Real Triangle::pdfSurface(const Ray &ray) const
{
    RayHit hit = checkHit(ray, 0, INFINITY);
    if (!hit.hit)
        return 0;

    // Compute the area of the triangle
    Real area = length<Real>(cross(v1 - v0, v2 - v0)) / 2;

    // Compute the pdf
    Real pdf = hit.t * hit.t / (dot(-ray.dir, hit.normal) * area);

    return pdf;
}