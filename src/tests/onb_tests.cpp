#include <gtest/gtest.h>
#include "../vector.h"

#include "../custom/onb.h"
#include "../custom/pcg.h"
#include "../custom/utils.h"

using namespace cu_utils;

TEST(ONB, OrthogonalNormalizedBasis) {
    // Test case 1: Check that the basis vectors are orthogonal and normalized
    Vector3 a(1, 0, 0);
    Vector3 b(0, 1, 0);
    Vector3 c(0, 0, 1);
    onb basis = onb();
    basis.axis[0] = a;
    basis.axis[1] = b;
    basis.axis[2] = c;
    EXPECT_NEAR(dot(basis.u(), basis.v()), 0.0f, 1e-6);
    EXPECT_NEAR(dot(basis.u(), basis.w()), 0.0f, 1e-6);
    EXPECT_NEAR(dot(basis.v(), basis.w()), 0.0f, 1e-6);
    EXPECT_NEAR(length(basis.u()), 1.0f, 1e-6);
    EXPECT_NEAR(length(basis.v()), 1.0f, 1e-6);
    EXPECT_NEAR(length(basis.w()), 1.0f, 1e-6);

    // Test case 2: Check that the local method returns the correct vector
    Vector3 v = basis.local(1, 2, 3);
    EXPECT_NEAR(v.x, 1.0f, 1e-6);
    EXPECT_NEAR(v.y, 2.0f, 1e-6);
    EXPECT_NEAR(v.z, 3.0f, 1e-6);

    // Case 3: Generate basis around various vectors and check that the basis is orthogonal and normalized
    Vector3 zvecs[] = {Vector3(0, 0, 1), Vector3(0, 1, 0), Vector3(1, 0, 0), Vector3(1, 1, 1), Vector3(1, 1, 0), Vector3(1, 0, 1), Vector3(0, 1, 1)};

    for (Vector3 vec : zvecs) {
        basis.build_from_w(vec);
        EXPECT_NEAR(dot(basis.u(), basis.v()), 0.0f, 1e-6);
        EXPECT_NEAR(dot(basis.u(), basis.w()), 0.0f, 1e-6);
        EXPECT_NEAR(dot(basis.v(), basis.w()), 0.0f, 1e-6);
        EXPECT_NEAR(length(basis.u()), 1.0f, 1e-6);
        EXPECT_NEAR(length(basis.v()), 1.0f, 1e-6);
        EXPECT_NEAR(length(basis.w()), 1.0f, 1e-6);
    }

    // Case 4: Check the same for a couple more random vectors
    Vector3 rvecs[] = {Vector3(0.1, 0.2, 0.3), Vector3(0.4, 0.5, 0.6), Vector3(0.7, 0.8, 0.9), Vector3(0.1, 0.4, 0.7), Vector3(0.2, 0.5, 0.8), Vector3(0.3, 0.6, 0.9)};
    for (Vector3 vec : rvecs) {
        basis.build_from_w(vec);
        EXPECT_NEAR(dot(basis.u(), basis.v()), 0.0f, 1e-6);
        EXPECT_NEAR(dot(basis.u(), basis.w()), 0.0f, 1e-6);
        EXPECT_NEAR(dot(basis.v(), basis.w()), 0.0f, 1e-6);
        EXPECT_NEAR(length(basis.u()), 1.0f, 1e-6);
        EXPECT_NEAR(length(basis.v()), 1.0f, 1e-6);
        EXPECT_NEAR(length(basis.w()), 1.0f, 1e-6);
    }
}

TEST(Utils, RandomCosineDirection) {
    // Test case 1: Check that the direction vector is in the hemisphere
    pcg32_state rng = init_pcg32(0, 0);
    for (int i = 0; i < 100; i++) {
        Vector3 normal = randomUnitVector(rng);
        Vector3 direction = random_cosine_direction(rng);

        onb basis = onb();
        basis.build_from_w(normal);

        // Transform the direction vector to the local coordinate system
        Vector3 local = basis.local(direction);

        EXPECT_TRUE(dot(local, normal) >= 0.0f);
        EXPECT_NEAR(length(direction), 1.0f, 1e-6);
    }

    // Test case 3: Check that the direction vector is oriented along the surface normal hemisphere
    Vector3 normal(0, 0, 1);
    Vector3 direction = random_cosine_direction(rng);

    Vector3 up(0, 1, 0);
    Vector3 right = cross(normal, up);
    up = cross(right, normal);
    onb basis = onb();
    basis.axis[0] = right;
    basis.axis[1] = up;
    basis.axis[2] = normal;

    Vector3 local = basis.local(direction);
    EXPECT_NEAR(local.z, sqrt(1 - pow(local.x, 2) - pow(local.y, 2)), 1e-6);
}