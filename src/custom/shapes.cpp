#include "shapes.h"
#include <iostream>

using namespace cu_utils;
using namespace std;

RayHit Sphere::checkHit(const Ray &ray) const
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

        if (t <= 0)
        {
            // Check the further hit point
            t = (-b + sqrt(discriminant)) / (2.0 * a);

            // normal is pointing the other way in this case
            n = -normalize(ray * t - center);
        }

        // Both potential hit points are behind the camera, invalid.
        if (t <= 0)
            return RayHit();

        // Valid rayhit found, returning
        return RayHit(true, t, this, n);
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
}

Triangle::Triangle(Vector3 v0, Vector3 v1, Vector3 v2, int material_id)
{
    this->v0 = v0;
    this->v1 = v1;
    this->v2 = v2;
    this->material_id = material_id;
}

RayHit Triangle::checkHit(const Ray &ray) const
{
    // Borrowed from RT in One Weekend
    Vector3 e1 = v1 - v0;
    Vector3 e2 = v2 - v0;
    Vector3 h = cross(ray.dir, e2);
    Real a = dot(e1, h);

    if (a > -0.00001 && a < 0.00001)
        return RayHit(); // Ray is parallel to triangle

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
    Vector3 n = normalize(cross(e1, e2));

    return RayHit(true, t, this, n);
}

Vector3 Triangle::getBarycentric(const Ray &ray, const RayHit &hit) const
{
    Vector3 p = ray * hit.t;

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