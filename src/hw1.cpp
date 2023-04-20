#include "hw1.h"
#include "hw1_scenes.h"
#include "custom/utils.h"
#include "custom/camera.h"
#include "custom/renderer.h"
#include "custom/ray.h"
#include <iostream>
#include <vector>

using namespace std;
using namespace hw1;

Image3 hw_1_1(const std::vector<std::string> & /*params*/)
{
    // Homework 1.1: generate camera rays and output the ray directions
    // The camera is positioned at (0, 0, 0), facing towards (0, 0, -1),
    // with an up vector (0, 1, 0) and a vertical field of view of 90 degree.

    Image3 img(640 /* width */, 480 /* height */);

    cu_utils::Camera camera = cu_utils::CameraBuilder(640, 480).build();

    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            cu_utils::Ray ray = camera.ScToWRay(x, y);

            img(x, y) = ray.dir;
        }
    }

    return img;
}

Image3 hw_1_2(const std::vector<std::string> & /*params*/)
{
    // Homework 1.2: intersect the rays generated from hw_1_1
    // with a unit sphere located at (0, 0, -2)

    // Ok this image is just flipped for some reason

    Image3 img(640 /* width */, 480 /* height */);
    cu_utils::Camera camera = cu_utils::CameraBuilder(640, 480).build();

    cu_utils::Renderer renderer(cu_utils::Mode::NORMAL);

    renderer.render(img, renderer.getTestScene());

    return img;
}

Image3 hw_1_3(const std::vector<std::string> &params)
{
    // Homework 1.3: add camera control to hw_1_2.
    // We will use a look at transform:
    // The inputs are "lookfrom" (camera position),
    //                "lookat" (target),
    //                and the up vector
    // and the vertical field of view (in degrees).
    // If the user did not specify, fall back to the default
    // values below.
    // If you use the default values, it should render
    // the same image as hw_1_2.

    Vector3 lookfrom = Vector3{0, 0, 0};
    Vector3 lookat = Vector3{0, 0, -2};
    Vector3 up = Vector3{0, 1, 0};
    Real vfov = 90;
    for (int i = 0; i < (int)params.size(); i++)
    {
        if (params[i] == "-lookfrom")
        {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            Real z = std::stof(params[++i]);
            lookfrom = Vector3{x, y, z};
        }
        else if (params[i] == "-lookat")
        {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            Real z = std::stof(params[++i]);
            lookat = Vector3{x, y, z};
        }
        else if (params[i] == "-up")
        {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            Real z = std::stof(params[++i]);
            up = Vector3{x, y, z};
        }
        else if (params[i] == "-vfov")
        {
            vfov = std::stof(params[++i]);
        }
    }

    // avoid unused warnings
    UNUSED(lookfrom);
    UNUSED(lookat);
    UNUSED(up);
    UNUSED(vfov);

    Image3 img(640 /* width */, 480 /* height */);

    // Render
    cu_utils::Renderer renderer(cu_utils::Mode::NORMAL);

    cu_utils::Scene scene = renderer.getTestScene();
    scene.camera.lookat = lookat;
    scene.camera.lookfrom = lookfrom;
    scene.camera.up = up;
    scene.camera.vfov = vfov;
    renderer.render(img, scene);

    return img;
}

Image3 hw_1_4(const std::vector<std::string> &params)
{
    // Homework 1.4: render the scenes defined in hw1_scenes.h
    // output their diffuse color directly.
    if (params.size() == 0)
    {
        return Image3(0, 0);
    }

    int scene_id = std::stoi(params[0]);
    UNUSED(scene_id); // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(640 /* width */, 480 /* height */);
    cu_utils::Renderer renderer(cu_utils::Mode::FLAT);
    renderer.render(img, hw1_scenes[scene_id]);

    return img;
}

Image3 hw_1_5(const std::vector<std::string> &params)
{
    // Homework 1.5: render the scenes defined in hw1_scenes.h,
    // light them using the point lights in the scene.
    if (params.size() == 0)
    {
        return Image3(0, 0);
    }

    int scene_id = std::stoi(params[0]);
    UNUSED(scene_id); // avoid unused warning
                      // Your scene is hw1_scenes[scene_id]

    Image3 img(640 /* width */, 480 /* height */);
    cu_utils::Renderer renderer(cu_utils::Mode::LAMBERT);

    renderer.render(img, hw1_scenes[scene_id]);

    return img;
}

Image3 hw_1_6(const std::vector<std::string> &params)
{
    // Homework 1.6: add antialiasing to homework 1.5
    if (params.size() == 0)
    {
        return Image3(0, 0);
    }

    int scene_id = 0;
    int spp = 64;
    for (int i = 0; i < (int)params.size(); i++)
    {
        if (params[i] == "-spp")
        {
            spp = std::stoi(params[++i]);
        }
        else
        {
            scene_id = std::stoi(params[i]);
        }
    }

    UNUSED(scene_id); // avoid unused warning
    UNUSED(spp);      // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(160 /* width */, 120 /* height */);
    cu_utils::Renderer renderer(cu_utils::Mode::LAMBERT);
    renderer.spp = spp;
    renderer.maxDepth = 0;

    renderer.render(img, hw1_scenes[scene_id]);

    return img;
}

Image3 hw_1_7(const std::vector<std::string> &params)
{
    // Homework 1.7: add mirror materials to homework 1.6
    if (params.size() == 0)
    {
        return Image3(0, 0);
    }

    int scene_id = 0;
    int spp = 64;
    for (int i = 0; i < (int)params.size(); i++)
    {
        if (params[i] == "-spp")
        {
            spp = std::stoi(params[++i]);
        }
        else
        {
            scene_id = std::stoi(params[i]);
        }
    }

    UNUSED(scene_id); // avoid unused warning
    UNUSED(spp);      // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    Image3 img(640 /* width */, 480 /* height */);

    cu_utils::Renderer renderer(cu_utils::Mode::MATTE_REFLECT);
    renderer.spp = spp;

    renderer.render(img, hw1_scenes[scene_id]);

    return img;
}

Image3 hw_1_8(const std::vector<std::string> &params)
{
    // Homework 1.8: parallelize HW 1.7
    if (params.size() == 0)
    {
        return Image3(0, 0);
    }

    int scene_id = 0;
    int spp = 64;
    for (int i = 0; i < (int)params.size(); i++)
    {
        if (params[i] == "-spp")
        {
            spp = std::stoi(params[++i]);
        }
        else
        {
            scene_id = std::stoi(params[i]);
        }
    }

    UNUSED(scene_id); // avoid unused warning
    UNUSED(spp);      // avoid unused warning
    // Your scene is hw1_scenes[scene_id]

    cu_utils::Renderer renderer(cu_utils::Mode::MATTE_REFLECT);
    renderer.spp = spp;
    renderer.maxDepth = 4;

    Image3 img(1280 /* width */, 960 /* height */);
    renderer.render(img, hw1_scenes[scene_id]);

    // Image3 img(512 /* width */, 384 /* height */);
    // renderer.render(img, cu_utils::Renderer::getCustomScene());

    return img;
}
