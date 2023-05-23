#include <gtest/gtest.h>
#include "../vector.h"
#include "../custom/ray.h"
#include "../custom/shapes.h"
#include "../custom/utils.h"
#include "../custom/materials.h"


using namespace cu_utils;

TEST(Utils, RandomUnitVector) {
    // Test case 1: Check that the generated vector is of unit length
    pcg32_state rng = init_pcg32(1, 12345);
    Vector3 v = randomUnitVector(rng);
    EXPECT_NEAR(length(v), 1.0f, 1e-6);

    // Test case 2: Check that the generated vectors are uniformly distributed
    const int numSamples = 100000;
    const float binSize = 0.1f;
    Vector3 sum = Vector3(0, 0, 0);
    for (int i = 0; i < numSamples; i++) {
        Vector3 v = randomUnitVector(rng);
        
        // Check that the vector is of unit length
        EXPECT_NEAR(length(v), 1.0f, 1e-6);

        // Add the vector to the sum
        sum += v;
    }

    // Check that the ave is close to zero
    EXPECT_NEAR(length(sum*(1.0/numSamples)), 0.0f, 0.01);
}

TEST(LambertMaterial, Scatter) {
    pcg32_state rng = init_pcg32(1, 12345);
    LambertMaterial material;
    Ray ray(Vector3(0, 0, 0), Vector3(1, 0, 0));
    RayHit hit;
    hit.t = 1.0f;
    hit.normal = Vector3(0, 1, 0);
    Vector3 alb;
    Ray scattered;
    Real pdf;
    bool result = material.scatter(ray, hit, alb, scattered, pdf, rng);
    EXPECT_TRUE(result);
    
    // Check that the scattered ray is in the hemisphere
    EXPECT_TRUE(dot(scattered.dir, hit.normal) > 0);

    // Reproduce a bunch of times and check that the average is mirrored around n
    const int numSamples = 100000;
    const float binSize = 0.1f;
    Vector3 sum = Vector3(0, 0, 0);

    for (int i = 0; i < numSamples; i++) {
        ray = Ray(Vector3(0, 0, 0), Vector3(1, 0, 0));
        hit.t = 1.0f;
        hit.normal = Vector3(0, -1, 0);
        bool result = material.scatter(ray, hit, alb, scattered, pdf, rng);
        EXPECT_TRUE(result);
        
        // Check that the scattered ray is in the hemisphere
        EXPECT_TRUE(dot(scattered.dir, hit.normal) > 0);

        // Add the vector to the sum
        sum += scattered.dir;
    }

    sum = normalize(sum);
    EXPECT_NEAR(dot(sum, hit.normal), 1.0f, 0.01);
}