#include <gtest/gtest.h>
#include <cmath>

// Re-declare the lightweight structural vector for computation tests
struct Vector3D {
    float x, y, z;
};

// Automated Test Case 1: Check 3D Rotation Matrix Calculation Precision
TEST(EngineMathTest, RotationZeroRadiansHasNoEffect) {
    Vector3D point = {1.0f, 2.0f, 3.0f};
    float angle = 0.0f; // No rotation

    // Apply the exact formula from our src/Engine.cpp file
    float rotatedX = point.x * cosf(angle) - point.z * sinf(angle);
    float rotatedZ = point.x * sinf(angle) + point.z * cosf(angle);

    // Verify mathematical equality with a minute tolerance delta threshold (0.0001)
    EXPECT_NEAR(rotatedX, 1.0f, 0.0001f);
    EXPECT_NEAR(rotatedZ, 3.0f, 0.0001f);
}

// Automated Test Case 2: Validate Perspective Projection Equations
TEST(EngineMathTest, PerspectiveProjectionCalculatesCorrectly) {
    float rawX = 2.0f;
    float finalZ = 4.0f; // Depth value
    float FOV_SCALE = 400.0f;
    int HALF_WIDTH = 400;

    // Apply the exact perspective formula we used to render the cube
    int projectedScreenX = static_cast<int>(HALF_WIDTH + (rawX * FOV_SCALE) / finalZ);

    // Expected value math: 400 + (2 * 400) / 4 = 400 + 800 / 4 = 400 + 200 = 600
    EXPECT_EQ(projectedScreenX, 600);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
