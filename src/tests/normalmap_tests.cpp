#include <gtest/gtest.h>
#include <filesystem>
#include "../vector.h"
#include "../image.h"

#include "../custom/ray.h"
#include "../custom/bounding_box.h"
#include "../custom/shapes.h"
#include "../custom/utils.h"
#include "../custom/sah.h"
#include "../custom/scene.h"
#include "../custom/renderer.h"
#include "../custom/onb.h"

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

TEST(ONBTest, TransformNormalMap) {
    // Define the normal and bitangent axes of the ONB
    // Make sure it is a RHS
    Vector3 normal(0.0f, 0.0f, 1.0f);
    Vector3 tangent(0.0f, 1.0f, 0.0f);
    Vector3 bitangent(-1.0f, 0.0f, 0.0f);

    // Create the ONB object
    // The basis isn't orthogonal at all but oh well
    onb basis;
    basis.axis[2] = normal;
    basis.axis[0] = tangent;
    basis.axis[1] = bitangent;

    Vector3 normalMap(0.0f, 0.0f, 1.0f); 

    Vector3 transformed = basis.local(normalMap);
    ASSERT_TRUE(equals(transformed, Vector3(0.0f, 0.0f, 1.0f)));

    // Check for some other orientations
    normalMap = Vector3(0.0f, 1.0f, 0.0f);
    transformed = basis.local(normalMap);
    ASSERT_TRUE(equals(transformed, Vector3(-1.0f, 0.0f, 0.0f)));

    normalMap = Vector3(1.0f, 0.0f, 0.0f);
    transformed = basis.local(normalMap);
    ASSERT_TRUE(equals(transformed, Vector3(0.0f, 1.0f, 0.0f)));
}

TEST(NormalMapTest, ScenePairedImport) {
    Scene scene = Scene::defaultScene();

    ParsedImageTexture *texMeta = new ParsedImageTexture();
    fs::path rel = fs::path("../custom_scenes/steel-groupers/textures/Fish_Color.jpg");
    fs::path normRel = fs::path("../custom_scenes/steel-groupers/textures/Fish_Color_NormalSmooth.png");
    fs::path wd = fs::current_path();

    texMeta->filename = wd/rel;

    // Check that the file exists
    ASSERT_TRUE(fs::exists(wd/normRel));

    // Perform a path splice


}