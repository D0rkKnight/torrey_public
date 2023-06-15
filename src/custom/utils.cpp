#include "utils.h"

using namespace cu_utils;

// Includes some of the bulkier util calls for rendering and whatnot
Real cu_utils::testingDot(const Vector3& a, const Vector3& b)
{
    return dot(a, b);
}

Image3 cu_utils::loadUnbiasedImage(const fs::path &path) {
    int w, h, n;
    stbi_uc* data = stbi_load(path.c_str(), &w, &h, &n, 3);
    Image3 image = Image3(w, h);
    int j = 0;
    for (int i = 0; i < w * h; i++) {
        image(i)[0] = data[j++] / 255.0;
        image(i)[1] = data[j++] / 255.0;
        image(i)[2] = data[j++] / 255.0;
    }
    stbi_image_free(data);

    return image;
}