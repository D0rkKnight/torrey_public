/**
 * @file renderer.h
 * The renderer takes an image and a camera and renders the image.
 * Also has an associated builder since configuring it is a bit tedious.
 */

#pragma once

#include "utils.h"
#include "camera.h"
#include "ray.h"
#include "pcg.h"
#include "../hw1_scenes.h"
#include <iostream>
#include <vector>
#include "scene.h"

#include "../parallel.h"

namespace cu_utils
{
    enum class Mode
    {
        NORMAL,
        OBJECT,
        FLAT,
        LAMBERT,
        MATTE_REFLECT, // Handles matte and reflective materials
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

        Renderer(Mode mode) : mode(mode)
        {
        }

        // Simple transformer for hw1 scenes
        void render(Image3 &img, const hw1::Scene &scene, int seed = 0)
        {
            render(img, Scene(scene), seed);
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

            constexpr int tile_size = 16;
            int num_tiles_x = (img.width + tile_size - 1) / tile_size;
            int num_tiles_y = (img.height + tile_size - 1) / tile_size;
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
                                
                                img(x,y) = renderPixel(img, scene, x, y, seed);
                            }
                            } },
                         Vector2i(num_tiles_x, num_tiles_y));
        }

        Vector3 renderPixel(Image3 &img, const Scene &scene, int x, int y, int seed = 0)
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
                return getPixelColor(ray, scene, maxDepth);
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
                    color += getPixelColor(ray, scene, maxDepth);
                }
                return color / (Real)spp;
            }
        }

        Vector3 getPixelColor(const Ray &ray, const Scene &scene, int depth = 0)
        {
            auto bestHit = castRay(ray, scene.shapes);

            Vector3 color = Vector3{0.5, 0.5, 0.5};
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
                    Vector3 diffuseColor = material.color;
                    color = diffuseColor;
                }
                break;

                case Mode::LAMBERT:

                { // Check every light in the scene
                    color = lambert(ray, bestHit, scene);
                }

                break;

                case Mode::MATTE_REFLECT:
                {
                    // Might want to just feed this into the lambert equation
                    Material material = scene.materials[bestHit.sphere->material_id];

                    if (material.type == MaterialType::Diffuse || depth <= 0)
                    {
                        color = lambert(ray, bestHit, scene);
                    }
                    else if (material.type == MaterialType::Mirror)
                    {
                        // Get the reflection direction
                        Vector3 hit = ray * bestHit.t;
                        Vector3 reflectDir = reflect(ray.dir, bestHit.normal);
                        Ray reflectRay = Ray(hit, reflectDir);

                        // Move the ray forward by 10^-4
                        reflectRay.origin += reflectRay.dir * 0.0001;

                        // Recurse
                        color = hadamard(material.color, getPixelColor(reflectRay, scene, depth - 1));
                    }
                }
                break;
                }
            }

            return color;
        }

        RayHit castRay(const Ray &ray, const std::vector<Shape> &spheres)
        {
            // Run against every sphere
            RayHit bestHit = RayHit();
            for (const Shape &sphere : spheres)
            {
                RayHit hit = cu_utils::hitSphere(sphere, ray);
                if (hit.t < 0)
                    continue;

                if (bestHit.t < 0 || hit.t < bestHit.t)
                {
                    bestHit = hit;
                }
            }

            return bestHit;
        }

        Vector3 lambert(const Ray ray, const RayHit bestHit, const Scene &scene)
        {
            Vector3 color = Vector3{0, 0, 0};
            Vector3 hit = ray * bestHit.t;

            for (const PointLight &light : scene.lights)
            {
                // Get the material from the scene
                Material material = scene.materials[bestHit.sphere->material_id];

                Vector3 albedo = material.color;
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

                RayHit shadowHit = castRay(shadowRay, scene.shapes);
                if (shadowHit.hit && shadowHit.t < distance(light.position, hit))
                {
                    // If the shadow ray hit something, then it's in shadow
                    contribution *= 0.0;
                }

                color += contribution;
            }

            return color;
        }

        static Scene
        getTestScene()
        {
            // Create a default scene
            hw1::Scene scene{

                hw1::Camera{
                    Vector3{0, 0, 0},
                    Vector3{0, 0, -1},
                    Vector3{0, 1, 0},
                    90,
                },
                std::vector<hw1::Sphere>{
                    {Vector3{0.0, 0.0, -2.0}, 1.0, 0},
                },
                std::vector<hw1::Material>{
                    {hw1::MaterialType::Diffuse, Vector3{0.75, 0.25, 0.25}},
                },
                std::vector<hw1::PointLight>{
                    {Vector3{100, 100, 100}, Vector3{5, 5, -2}},
                },
            };

            return Scene(scene);
        }

        static Scene
        getCustomScene()
        {
            // Massive array of spheres
            hw1::Scene scene{

                hw1::Camera{
                    Vector3{0, 0, -1},
                    Vector3{0, 0, -2},
                    Vector3{0, 1, 0},
                    90,
                },
                std::vector<hw1::Sphere>{},
                std::vector<hw1::Material>{
                    {hw1::MaterialType::Mirror, Vector3{0.75, 0.25, 0.25}},
                    {hw1::MaterialType::Mirror, Vector3{0.25, 0.75, 0.25}},
                    {hw1::MaterialType::Mirror, Vector3{0.25, 0.25, 0.75}},
                },
                std::vector<hw1::PointLight>{
                    {Vector3{100, 100, 100}, Vector3{0, 0, -3}},
                },
            };

            // Create a bunch of spheres
            for (int i = 0; i < 20; i += 2)
            {
                for (int j = 0; j < 20; j += 2)
                {
                    for (int k = 0; k < 20; k += 2)
                    {
                        hw1::Sphere sphere{
                            Vector3{(Real)i - 10, (Real)j - 10, (Real)k - 10},
                            0.5,
                            (i + j + k) % 3,
                        };

                        scene.shapes.push_back(sphere);
                    }
                }
            }

            return Scene(scene);
        }
    };

    class RendererBuilder
    {
    };
}