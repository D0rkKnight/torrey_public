#include "hw2.h"
#include "parse_scene.h"
#include "print_scene.h"
#include "timer.h"

#include "custom/scene.h"
#include "custom/renderer.h"

Image3 hw_2_1(const std::vector<std::string> &params)
{
    // Homework 2.1: render a single triangle and outputs
    // its barycentric coordinates.
    // We will use the following camera parameter
    // lookfrom = (0, 0,  0)
    // lookat   = (0, 0, -1)
    // up       = (0, 1,  0)
    // vfov     = 45
    // and we will parse the triangle vertices from params
    // The three vertices are stored in v0, v1, and v2 below.

    std::vector<float> tri_params;
    int spp = 16;
    for (int i = 0; i < (int)params.size(); i++)
    {
        if (params[i] == "-spp")
        {
            spp = std::stoi(params[++i]);
        }
        else
        {
            tri_params.push_back(std::stof(params[i]));
        }
    }

    if (tri_params.size() < 9)
    {
        // Not enough parameters to parse the triangle vertices.
        return Image3(0, 0);
    }

    Vector3 p0{tri_params[0], tri_params[1], tri_params[2]};
    Vector3 p1{tri_params[3], tri_params[4], tri_params[5]};
    Vector3 p2{tri_params[6], tri_params[7], tri_params[8]};

    Image3 img(640 /* width */, 480 /* height */);

    cu_utils::Scene scene = cu_utils::Scene::defaultScene();
    scene.shapes.push_back(new cu_utils::Triangle(p0, p1, p2, 0));
    scene.camera.lookfrom = Vector3{0, 0, 0};
    scene.camera.lookat = Vector3{0, 0, -1};
    scene.camera.up = Vector3{0, 1, 0};
    scene.camera.vfov = 45;

    cu_utils::Renderer renderer(cu_utils::Mode::BARYCENTRIC);
    renderer.spp = spp;

    renderer.render(img, scene);

    return img;
}

Image3 hw_2_2(const std::vector<std::string> &params)
{
    // Homework 2.2: render a triangle mesh.
    // We will use the same camera parameter:
    // lookfrom = (0, 0,  0)
    // lookat   = (0, 0, -1)
    // up       = (0, 1,  0)
    // vfov     = 45
    // and we will use a fixed triangle mesh: a tetrahedron!
    int spp = 16;
    for (int i = 0; i < (int)params.size(); i++)
    {
        if (params[i] == "-spp")
        {
            spp = std::stoi(params[++i]);
        }
    }

    std::vector<Vector3> positions = {
        Vector3{0.0, 0.5, -2.0},
        Vector3{0.0, -0.3, -1.0},
        Vector3{1.0, -0.5, -3.0},
        Vector3{-1.0, -0.5, -3.0}};
    std::vector<Vector3i> indices = {
        Vector3i{0, 1, 2},
        Vector3i{0, 3, 1},
        Vector3i{0, 2, 3},
        Vector3i{1, 2, 3}};

    Image3 img(640 /* width */, 480 /* height */);

    cu_utils::Scene scene = cu_utils::Scene::defaultScene();
    scene.camera.lookfrom = Vector3{0, 0, 0};
    scene.camera.lookat = Vector3{0, 0, -1};
    scene.camera.up = Vector3{0, 1, 0};
    scene.camera.vfov = 45;

    for (int i = 0; i < (int)indices.size(); i++)
    {
        scene.shapes.push_back(new cu_utils::Triangle(
            positions[indices[i][0]],
            positions[indices[i][1]],
            positions[indices[i][2]],
            i));
    }

    cu_utils::Renderer renderer(cu_utils::Mode::BARYCENTRIC);
    renderer.spp = spp;

    renderer.render(img, scene);

    return img;
}

Image3 hw_2_3(const std::vector<std::string> &params)
{
    // Homework 2.3: render a scene file provided by our parser.
    if (params.size() < 1)
    {
        return Image3(0, 0);
    }

    Timer timer;
    tick(timer);
    ParsedScene scene = parse_scene(params[0]);
    std::cout << "Scene parsing done. Took " << tick(timer) << " seconds." << std::endl;
    std::cout << scene << std::endl;

    cu_utils::Renderer renderer(cu_utils::Mode::MATTE_REFLECT);
    renderer.maxDepth = 3;

    return renderer.render(scene);
}

Image3 hw_2_4(const std::vector<std::string> &params)
{
    // Homework 2.4: render the AABBs of the scene.
    if (params.size() < 1)
    {
        return Image3(0, 0);
    }

    Timer timer;
    tick(timer);
    ParsedScene scene = parse_scene(params[0]);
    std::cout << "Scene parsing done. Took " << tick(timer) << " seconds." << std::endl;
    UNUSED(scene);

    cu_utils::Renderer renderer(cu_utils::Mode::AABB);

    return renderer.render(scene);
}

Image3 hw_2_5(const std::vector<std::string> &params)
{
    // Homework 2.5: rendering with BVHs
    if (params.size() < 1)
    {
        return Image3(0, 0);
    }

    Timer timer;
    tick(timer);
    ParsedScene scene = parse_scene(params[0]);
    std::cout << "Scene parsing done. Took " << tick(timer) << " seconds." << std::endl;
    UNUSED(scene);

    cu_utils::Renderer renderer(cu_utils::Mode::MATTE_REFLECT);

    return renderer.render(scene);
}
