#pragma once
#include "../vector.h"

class onb {
    public:
        onb() {}

        inline Vector3 operator[](int i) const { return axis[i]; }

        Vector3 u() const { return axis[0]; }
        Vector3 v() const { return axis[1]; }
        Vector3 w() const { return axis[2]; }

        Vector3 local(double a, double b, double c) const {
            return a*u() + b*v() + c*w();
        }

        Vector3 local(const Vector3& a) const {
            return a.x*u() + a.y*v() + a.z*w();
        }

        void build_from_w(const Vector3&);

    public:
        Vector3 axis[3];
};