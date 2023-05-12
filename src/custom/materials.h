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

        Vector3 getTexColor(Real u, Real v);
        void loadTexture(ParsedImageTexture *texMeta);

        virtual Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, pcg32_state &rng, int depth) const;
    };

    struct LambertMaterial : public Material
    {
        LambertMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, pcg32_state &rng, int depth) const override;
    };

    struct MirrorMaterial : public Material
    {
        MirrorMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, pcg32_state &rng, int depth) const override;
    };

    struct PlasticMaterial : public Material
    {
        PlasticMaterial();
        Vector3 shadePoint(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, pcg32_state &rng, int depth) const override;
    };

    Vector3 lambert(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, pcg32_state &rng);
    Vector3 mirror(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, pcg32_state &rng, int depth);
    Vector3 plastic(const Renderer *renderer, const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, pcg32_state &rng, int depth);
}