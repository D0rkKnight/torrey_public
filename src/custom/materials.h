#pragma once
#include "../vector.h"
#include "../parse_scene.h"
#include "ray.h"
#include "bounding_box.h"
#include "pcg.h"

namespace cu_utils
{
    class Scene;
    class Renderer;

    struct Material
    {
        Vector3 flatColor;
        Scene *scene;

        // Just for plastics
        Real eta;

        // Can have a backing image texture.
        ParsedImageTexture *texMeta;

        Material();

        Vector3 getTexColor(Real u, Real v) const;
        void loadTexture(ParsedImageTexture *texMeta);

        virtual Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const;

        // Guessing that if albedo is 0, just don't scatter at all
        // Scatter handles the actual generation of sampling rays
        virtual bool scatter(const Ray &ray, const RayHit &hit, Vector3 &albedo, Ray &scattered, Real &pdf, pcg32_state &rng) const { return false; };

        // Given a sampling ray, generates the contribution weight for that ray
        // Will be pointy in BRDFs
        virtual Real scattering_pdf(const Ray &ray, const RayHit &hit, const Ray &scattered) const { return 0; };

        // Locks in values for child materials
        virtual void finish() {};
    };

    struct LambertMaterial : public Material
    {
        LambertMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const override;

        bool scatter(const Ray &ray, const RayHit &hit, Vector3 &albedo, Ray &scattered, Real &pdf, pcg32_state &rng) const;
        Real scattering_pdf(const Ray &ray, const RayHit &hit, const Ray &scattered) const;
    };

    struct MirrorMaterial : public Material
    {
        MirrorMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const override;
    };

    struct PlasticMaterial : public Material
    {
        // Time to become inspired
        LambertMaterial backingLambert;

        PlasticMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const override;
        void finish() override;
    };

    struct PhongMaterial : public Material
    {
        Real exp = 1;

        PhongMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const override;

        bool scatter(const Ray &ray, const RayHit &hit, Vector3 &albedo, Ray &scattered, Real &pdf, pcg32_state &rng) const;
        Real scattering_pdf(const Ray &ray, const RayHit &hit, const Ray &scattered) const;
    };

    struct BlinnPhongMaterial : public Material
    {
        Real exp = 1;

        BlinnPhongMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const override;

        bool scatter(const Ray &ray, const RayHit &hit, Vector3 &albedo, Ray &scattered, Real &pdf, pcg32_state &rng) const;
        Real scattering_pdf(const Ray &ray, const RayHit &hit, const Ray &scattered) const;
    };

    struct MicrofacetMaterial : public Material
    {
        Real exp = 1;

        MicrofacetMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth) const override;

        bool scatter(const Ray &ray, const RayHit &hit, Vector3 &albedo, Ray &scattered, Real &pdf, pcg32_state &rng) const;
        Real scattering_pdf(const Ray &ray, const RayHit &hit, const Ray &scattered) const;
    };

    Vector3 matte(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth);
    Vector3 mirror(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth);
    Vector3 plastic(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth);
    // Vector3 phong(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth);

}
