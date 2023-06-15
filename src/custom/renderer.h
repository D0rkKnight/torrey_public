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

            // Retrieve skybox
            // Just hardcode it, I don't care anymore
            scene.skybox = imread3("../custom_scenes/steel-groupers/textures/skybox.png");
            std::cout << "Loaded skybox" << std::endl;

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
            BVHNode root = BVHNode::buildTree(scene.shapes);
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

                            // Initialize pcg random number generator
                            pcg32_state rng = init_pcg32(1, seed);

                            // Render step
                            for (int y = y0; y < y1; y++) {
                            for (int x = x0; x < x1; x++) {
                                
                                img(x,y) = renderPixel(img, scene, root, x, y, rng);
                            }
                            } 
                            
                            reporter.update(1); },
                         Vector2i(num_tiles_x, num_tiles_y));

            reporter.done();
        }

        Vector3 renderPixel(Image3 &img, const Scene &scene, BVHNode &objRoot, int x, int y, pcg32_state &rng)
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
                return getPixelColor(ray, scene, objRoot, rng, maxDepth);
            }

            else
            {
                // Shoot multiple rays per pixel
                Vector3 color = Vector3{0, 0, 0};

                for (int i = 0; i < spp; i++)
                {
                    // Shoot a ray through a random point in the pixel
                    Real offX = next_pcg32_real<Real>(rng);
                    Real offY = next_pcg32_real<Real>(rng);
                    Ray ray = cam.ScToWRay(x + offX, y + offY);
                    color += getPixelColor(ray, scene, objRoot, rng, maxDepth);
                }
                return color / (Real)spp;
            }
        }

        Vector3 getPixelColor(const Ray &ray, const Scene &scene, const BVHNode &objRoot, pcg32_state &rng, int depth = 0) const
        {
            auto bestHit = castRay(ray, scene.shapes, objRoot);

            Vector3 color = bgCol;

            // Sample the skybox if we hit nothing (this is dangerous, but oh well.)
            if (bestHit.hit == 0)
            {
                color = sampleSkybox(ray.dir, scene.skybox);
            }

            else
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
                    Material *material = scene.materials[bestHit.sphere->material_id];

                    // Get the diffuse color from the mat
                    Vector3 diffuseColor = material->flatColor;
                    color = diffuseColor;
                }
                break;

                case Mode::LAMBERT:

                { // Check every light in the scene
                    color = matte(this, ray, bestHit, scene, objRoot, rng, depth);
                }

                break;

                case Mode::MATTE_REFLECT:
                {

                    if (bestHit.sphere->material_id < 0) {
                        if (bestHit.sphere->areaLight != nullptr) {
                            color = bestHit.sphere->areaLight->intensity;
                        }
                        break;
                    }

                    Material *material = scene.materials[bestHit.sphere->material_id];
                    color = material->shadePoint(this, ray, bestHit, scene, objRoot, rng, depth);
                }
                break;
                case Mode::BARYCENTRIC:
                {
                    if (const Triangle *tri = dynamic_cast<const Triangle *>(bestHit.sphere))
                    {
                        Vector3 bary = tri->getBarycentric(ray * bestHit.t);
                        color = bary;
                    }
                    else // Not a triangle, just render as flat
                    {
                        Material *material = scene.materials[bestHit.sphere->material_id];

                        // Get the diffuse color from the mat
                        Vector3 diffuseColor = material->flatColor;
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

        RayHit castRay(const Ray &ray, const vector<Shape *> &shapes, const BVHNode &objRoot) const
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
                    return RayHit{true, 1, shape, Vector3{0, 0, 0}, 0, 0, false};
                }

                return bestHit;
            }

            // What it's supposed to do: Check the object tree and render
            BVHNode::boxesHit = 0;
            BVHNode::scansMade = 0;
            RayHit bestHit = objRoot.checkHit(ray, 0, std::numeric_limits<Real>::max());

            // std::cout << "Boxes hit: " << BBNode::boxesHit << std::endl;
            // std::cout << "Scans made: " << BBNode::scansMade << std::endl;
            // std::cout << "Cull ratio: " << (Real)BBNode::boxesHit / (Real)BBNode::scansMade << std::endl;

            return bestHit;
        }

        // Sample the skybox given a ray and skybox texture
        Vector3 sampleSkybox(const Vector3& ray, const Image3& skybox) const {
            float u, v;
            float absRx = std::abs(ray.x);
            float absRy = std::abs(ray.y);
            float absRz = std::abs(ray.z);

            int faceIndex;
            if (absRx >= absRy && absRx >= absRz) {
                // Right or left face
                faceIndex = (ray.x > 0) ? 0 : 1;
                u = -ray.z / absRx;
                v = -ray.y / absRx;
            } else if (absRy >= absRx && absRy >= absRz) {
                // Up or down face
                faceIndex = (ray.y > 0) ? 2 : 3;
                u = ray.x / absRy;
                v = ray.z / absRy;
            } else {
                // Front or back face
                faceIndex = (ray.z > 0) ? 4 : 5;
                u = -ray.x / absRz;
                v = -ray.y / absRz;
            }

            int faceOffset = 0;
            switch(faceIndex) {
                case 0: faceOffset = 7; break;
                case 1: faceOffset = 5; break;
                case 2: faceOffset = 2; break;
                case 3: faceOffset = 10; break;
                case 4: faceOffset = 6; break;
                case 5: faceOffset = 4; break;
            }

            // Calculate the pixel coordinates in the selected face of the skybox texture
            int faceWidth = skybox.width / 4; // Assuming the skybox has a cross shape with equal-sized faces
            int faceHeight = skybox.height / 3;

            int pixelX = static_cast<int>((u + 1.0f) * (faceWidth - 1) / 2.0f + faceOffset % 4 * faceWidth);
            int pixelY = static_cast<int>((v + 1.0f) * (faceHeight - 1) / 2.0f + faceOffset / 4 * faceHeight);

            // Calculate the starting index of the selected face in the skybox texture
            // int faceStartIndex = faceIndex * faceWidth * faceHeight;

            // Access the corresponding pixel in the skybox texture
            // int pixelIndex = faceStartIndex + (pixelY * faceWidth + pixelX);
            Vector3 col = skybox(pixelX, pixelY);

            return col;
        }
    };

    class RendererBuilder
    {
    };
}