#include "onb.h"

void onb::build_from_w(const Vector3& n) {
    axis[2] = normalize(n);
    Vector3 a = (fabs(w().x) > 0.9) ? Vector3(0,1,0) : Vector3(1,0,0);
    axis[1] = normalize(cross(w(), a));
    axis[0] = cross(w(), v());
}