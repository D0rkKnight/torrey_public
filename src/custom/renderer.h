/**
 * @file renderer.h
 * The renderer takes an image and a camera and renders the image.
 * Also has an associated builder since configuring it is a bit tedious.
 */

#pragma once

#include "utils.h"
#include "camera.h"
#include "ray.h"
#include "bounding_box.h"

#include "pcg.h"
#include <iostream>
#include <vector>
#include "scene.h"

#include "../parallel.h"
#include "../parse_scene.h"
#include "../progressreporter.h"

namespace cu_utils
{
    enum class Mode
    {
        NORMAL,
        OBJECT,
        FLAT,
        LAMBERT,
        MATTE_REFLECT, // Handles matte and reflective materials
        BARYCENTRIC,   // Renders triangles as barycentric and everything else as flat
        AABB,
    };

    /**
     * @brief Handles rendering behavior. Scene data held by Scene object.
     *
     */
    class Renderer
    {
    public:
        Mode mode;
        int spp = 1;
        int maxDepth = 1;
        Vector3 bgCol = Vector3(0.5, 0.5, 0.5);

        Renderer(Mode mode) : mode(mode)
        {
        }

        Image3 render(const ParsedScene &parsed, int seed = 0)
        {

            spp = parsed.samples_per_pixel;
            std::cout << "spp overriden to " << spp << std::endl;

            bgCol = parsed.background_color;
            std::cout << "bgCol overriden to " << bgCol << std::endl;

            Image3 img(parsed.camera.width, parsed.camera.height);
            Scene scene(parsed);
            render(img, scene, seed);

            return img;
        }

        void render(Image3 &img, const Scene &scene, int seed = 0)
        {
            // Build better camera with scene data
            Camera cam = CameraBuilder(img.width, img.height)
                             .setLookFrom(scene.camera.lookfrom)
                             .setLookAt(scene.camera.lookat)
                             .setUp(scene.camera.up)
                             .setFov(scene.camera.vfov)
                             .build();

            // Build object hierarchy
            BBNode root = BBNode::buildTree(scene.shapes);
            std::cout << "Built object hierarchy" << std::endl;

            constexpr int tile_size = 16;
            int num_tiles_x = (img.width + tile_size - 1) / tile_size;
            int num_tiles_y = (img.height + tile_size - 1) / tile_size;

            ProgressReporter reporter(num_tiles_x * num_tiles_y);

            parallel_for([&](const Vector2i &tile)
                         {
                            int seed = tile[1] * num_tiles_x + tile[0];

                            // Clamping
                            int x0 = tile[0] * tile_size;
                            int x1 = std::min(x0 + tile_size, img.width);
                            int y0 = tile[1] * tile_size;
                            int y1 = std::min(y0 + tile_size, img.height);

                            // Render step
                            for (int y = y0; y < y1; y++) {
                            for (int x = x0; x < x1; x++) {
                                
                                img(x,y) = renderPixel(img, scene, root, x, y, seed);
                            }
                            } 
                            
                            reporter.update(1); },
                         Vector2i(num_tiles_x, num_tiles_y));

            reporter.done();
        }

        Vector3 renderPixel(Image3 &img, const Scene &scene, BBNode &objRoot, int x, int y, int seed = 0)
        {
            // Build better camera with scene data
            Camera cam = CameraBuilder(img.width, img.height)
                             .setLookFrom(scene.camera.lookfrom)
                             .setLookAt(scene.camera.lookat)
                             .setUp(scene.camera.up)
                             .setFov(scene.camera.vfov)
                             .build();

            // Just shoot through the center so it's deterministic if we have 1 spp
            if (spp == 1)
            {
                Ray ray = cam.ScToWRay(x + 0.5, y + 0.5);
                return getPixelColor(ray, scene, objRoot, maxDepth);
            }

            else
            {
                // Shoot multiple rays per pixel
                Vector3 color = Vector3{0, 0, 0};
                // Initialize pcg random number generator
                pcg32_state rng = init_pcg32(1, seed);

                for (int i = 0; i < spp; i++)
                {
                    // Shoot a ray through a random point in the pixel
                    Real offX = next_pcg32_real<Real>(rng);
                    Real offY = next_pcg32_real<Real>(rng);
                    Ray ray = cam.ScToWRay(x + offX, y + offY);
                    color += getPixelColor(ray, scene, objRoot, maxDepth);
                }
                return color / (Real)spp;
            }
        }

        Vector3 getPixelColor(const Ray &ray, const Scene &scene, const BBNode &objRoot, int depth = 0)
        {
            auto bestHit = castRay(ray, scene.shapes, objRoot);

            Vector3 color = bgCol;
            if (bestHit.hit > 0)
            {
                // Just a default value
                color = Vector3{1, 0, 0};

                switch (mode)
                {
                case Mode::NORMAL:
                {
                    color = (bestHit.normal + Vector3{1, 1, 1}) * 0.5;
                }
                break;
                case Mode::OBJECT:
                {
                    color = Vector3{1, 0, 0};
                }
                break;
                case Mode::FLAT:

                { // Get the material from the scene
                    Material material = scene.materials[bestHit.sphere->material_id];

                    // Get the diffuse color from the mat
                    Vector3 diffuseColor = material.flatColor;
                    color = diffuseColor;
                }
                break;

                case Mode::LAMBERT:

                { // Check every light in the scene
                    color = lambert(ray, bestHit, scene, objRoot);
                }

                break;

                case Mode::MATTE_REFLECT:
                {
                    // Might want to just feed this into the lambert equation
                    Material material = scene.materials[bestHit.sphere->material_id];

                    if (material.type == MaterialType::Diffuse || depth <= 0)
                    {
                        color = lambert(ray, bestHit, scene, objRoot);
                    }
                    else if (material.type == MaterialType::Mirror)
                    {
                        color = mirror(ray, bestHit, scene, objRoot, depth);
                    }
                    else if (material.type == MaterialType::Plastic)
                    {
                        color = plastic(ray, bestHit, scene, objRoot, depth);
                    }
                    // else if (material.type == MaterialType::Glass)
                    // {
                    //     color = glass(ray, bestHit, scene, objRoot, depth);
                    // }
                }
                break;
                case Mode::BARYCENTRIC:
                {
                    if (const Triangle *tri = dynamic_cast<const Triangle *>(bestHit.sphere))
                    {
                        Vector3 bary = tri->getBarycentric(ray, bestHit);
                        color = bary;
                    }
                    else // Not a triangle, just render as flat
                    {
                        Material material = scene.materials[bestHit.sphere->material_id];

                        // Get the diffuse color from the mat
                        Vector3 diffuseColor = material.flatColor;
                        color = diffuseColor;
                    }
                }
                break;
                case Mode::AABB:
                {
                    // Collision should be with a bounding box, just mark as white
                    color = Vector3{1, 1, 1};
                }
                }
            }

            return color;
        }

        RayHit castRay(const Ray &ray, const vector<Shape *> &shapes, const BBNode &objRoot)
        {
            // AABB Mode only behavior
            if (mode == Mode::AABB)
            {
                // Run against every shape
                RayHit bestHit = RayHit();
                for (const Shape *shape : shapes)
                {
                    BoundingBox bounds = shape->getBoundingBox();

                    // Check if the ray intersects the bounding box
                    if (!bounds.checkHit(ray))
                        continue;

                    // Some unique AABB behavior
                    // Just dump out a dummy hit for the AABB renderer
                    return RayHit{true, 1, shape, Vector3{0, 0, 0}, 0, 0};
                }

                return bestHit;
            }

            // What it's supposed to do: Check the object tree and render
            BBNode::boxesHit = 0;
            BBNode::scansMade = 0;
            RayHit bestHit = objRoot.checkHit(ray);

            // std::cout << "Boxes hit: " << BBNode::boxesHit << std::endl;
            // std::cout << "Scans made: " << BBNode::scansMade << std::endl;
            // std::cout << "Cull ratio: " << (Real)BBNode::boxesHit / (Real)BBNode::scansMade << std::endl;

            return bestHit;
        }

        Vector3 lambert(const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot)
        {
            Material material = scene.materials[bestHit.sphere->material_id];

            Vector3 color = Vector3{0, 0, 0};
            Vector3 hit = ray * bestHit.t;

            for (const PointLight &light : scene.lights)
            {
                // Use 0 0 uv since we don't have the uv coord from the ray hit yet
                Vector3 albedo = material.getColor(bestHit.u, bestHit.v);
                Vector3 lightDir = normalize(light.position - hit);

                Real diffuse = std::max(dot(lightDir, bestHit.normal), 0.0);
                Vector3 contribution = albedo * diffuse;

                contribution /= c_PI;

                Real distSqrd = distance_squared(light.position, hit);
                Vector3 attenuation = light.intensity / distSqrd;

                contribution = Vector3{contribution.x * attenuation.x, contribution.y * attenuation.y, contribution.z * attenuation.z};

                // Shadow cast
                Ray shadowRay = Ray(hit, lightDir);
                // Move the ray forward by 10^-4
                shadowRay.origin += shadowRay.dir * 0.0001;

                RayHit shadowHit = castRay(shadowRay, scene.shapes, objRoot);
                if (shadowHit.hit && shadowHit.t < distance(light.position, hit))
                {
                    // If the shadow ray hit something, then it's in shadow
                    contribution *= 0.0;
                }

                color += contribution;
            }

            return color;
        }

        Vector3 mirror(const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth)
        {
            Material material = scene.materials[bestHit.sphere->material_id];

            // Get the reflection direction
            Vector3 hit = ray * bestHit.t;
            Vector3 reflectDir = reflect(ray.dir, bestHit.normal);
            Ray reflectRay = Ray(hit, reflectDir);

            // Move the ray forward by 10^-4
            reflectRay.origin += reflectRay.dir * 0.0001;

            // Recurse
            Vector3 fresnel = fresnelSchlick(material.flatColor, bestHit.normal, reflectDir);

            return hadamard(fresnel, getPixelColor(reflectRay, scene, objRoot, depth - 1));
        }

        Vector3 plastic(const Ray ray, const RayHit bestHit, const Scene &scene, const BBNode &objRoot, int depth)
        {
            Material material = scene.materials[bestHit.sphere->material_id];

            // Get the reflection direction
            Vector3 hit = ray * bestHit.t;
            Vector3 reflectDir = reflect(ray.dir, bestHit.normal);
            Ray reflectRay = Ray(hit, reflectDir);

            // Move the ray forward by 10^-4
            reflectRay.origin += reflectRay.dir * 0.0001;
            Vector3 albedo = material.getColor(bestHit.u, bestHit.v);

            // Recurse
            Vector3 fresnel = fresnelSchlick(albedo, bestHit.normal, reflectDir);

            Vector3 reflectColor = getPixelColor(reflectRay, scene, objRoot, depth - 1);

            // Get the diffuse color from the mat

            // Get the diffuse contribution
            Vector3 diffuse = albedo * (1.0 - fresnel);

            // Get the specular contribution
            Vector3 specular = hadamard(fresnel, reflectColor);

            return diffuse + specular;
        }
    };

    class RendererBuilder
    {
    };
}