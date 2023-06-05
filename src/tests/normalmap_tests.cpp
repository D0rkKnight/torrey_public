#include <gtest/gtest.h>
#include <filesystem>
#include "../vector.h"
#include "../custom/ray.h"
#include "../custom/bounding_box.h"
#include "../custom/shapes.h"
#include "../custom/utils.h"
#include "../custom/sah.h"
#include "../image.h"
#include "../custom/scene.h"
#include "../custom/renderer.h"

using namespace cu_utils;

TEST(NormalMapTest, LoadNormalMap) {

    // Use absolute path
    fs::path workingDir = fs::current_path();
    fs::path normalMapPath = fs::path("../custom_scenes/steel-groupers/textures/Fish_NormalSmooth.png");
    fs::path absolute = workingDir / normalMapPath;

    Image3 normalMap = imread3(absolute);

    EXPECT_EQ(normalMap.width, 1024);
    EXPECT_EQ(normalMap.height, 1024);

    // Write image3 to file
    fs::path outPath = fs::path("../out/tests/normal_confirm.exr");

    imwrite(outPath, normalMap);
}

TEST(RenderTest, Render1) {
    fs::path workingDir = fs::current_path();
    fs::path scenePath = fs::path("../custom_scenes/steel-groupers/groupers.xml");
    ParsedScene scene = parse_scene(workingDir / scenePath);
    scene.samples_per_pixel = 5;

    cu_utils::Renderer renderer(cu_utils::Mode::MATTE_REFLECT);
    renderer.maxDepth = 10;

    Image3 out = renderer.render(scene);

    ASSERT_TRUE(true);
}