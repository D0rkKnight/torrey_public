#pragma once

#include "utils.h"
#include "../matrix.h"
#include "../vector.h"
#include "ray.h"

#include <iostream>

using namespace std;
namespace cu_utils
{
    class Camera
    {
    public:
        // Might want to break into a builder pattern
        Camera(int width, int height, Real near, Real far, Real fov, Vector3 lookFrom, Vector3 lookAt, Vector3 up, Real focalLen)
            : m_screenWidth(width), m_screenHeight(height), m_fov(fov), m_near(near), m_far(far), m_lookFrom(lookFrom), m_lookAt(lookAt), m_up(up), m_focalLen(focalLen)
        {
            aspectRatio = (Real)width / (Real)height;

            // vpHeight is a function the fov
            Real fovRads = d2r(fov);
            vpHeight = 2 * tan(fovRads / 2);
            vpWidth = aspectRatio * vpHeight;

            // Get orthonormal basis for camera transform (view space -> world space)
            Vector3 w = normalize(lookFrom - lookAt); // Oh w points backwards.
            Vector3 u = normalize(cross(up, w));
            Vector3 v = cross(w, u);

            // Get camera transform
            m_camToWorld = Matrix4x4{
                u.x,
                v.x,
                w.x,
                0.0,
                u.y,
                v.y,
                w.y,
                0.0,
                u.z,
                v.z,
                w.z,
                0.0,
                0.0,
                0.0,
                0.0,
                1.0,
            };
        }

        Ray ScToWRay(Real x, Real y) const
        {
            Vector2 normScreenPos = Vector2{(x / m_screenWidth),
                                            (y / m_screenHeight)};

            Vector2 vpPos = Vector2{((normScreenPos.x - 0.5) * vpWidth),
                                    ((normScreenPos.y - 0.5) * vpHeight)};

            // Position of camera is the point of ray convergence
            // I think you can just reflect the ray and it will work
            Vector3 sRayDir = normalize(Vector3(vpPos.x, -vpPos.y, -m_focalLen));

            // Transform ray to world space
            Vector4 wRayDir = matXvec(m_camToWorld, Vector4(sRayDir.x, sRayDir.y, sRayDir.z, 1.0));

            return Ray(m_lookFrom, Vector3(wRayDir.x, wRayDir.y, wRayDir.z));
            // return Ray(m_lookFrom, sRayDir);
        }

        // Camera values are protected since they are functions of each other
        Real GetScreenWidth() const
        {
            return m_screenWidth;
        }
        Real GetScreenHeight() const
        {
            return m_screenHeight;
        }

        Real GetFov() const
        {
            return m_fov;
        }
        Real GetNear() const
        {
            return m_near;
        }
        Real GetFar() const
        {
            return m_far;
        }

        Real GetVpWidth() const
        {
            return vpWidth;
        }
        Real GetVpHeight() const
        {
            return vpHeight;
        }
        Real GetAspectRatio() const
        {
            return aspectRatio;
        }

        Vector3 GetLookFrom() const
        {
            return m_lookFrom;
        }
        Vector3 GetLookAt() const
        {
            return m_lookAt;
        }
        Vector3 GetUp() const
        {
            return m_up;
        }

        Real GetFocalLen() const
        {
            return m_focalLen;
        }

    private:
        Real m_fov;
        Real m_near;
        Real m_far;

        Real vpWidth;
        Real vpHeight;
        Real aspectRatio;

        Vector3 m_lookFrom;
        Vector3 m_lookAt;
        Vector3 m_up;

        Matrix4x4 m_camToWorld;

        // These don't really get used but you need them to calculate aspect ratio.
        Real m_screenWidth;
        Real m_screenHeight;
        Real m_focalLen;
    };

    class CameraBuilder
    {
    public:
        CameraBuilder(int width, int height)
            : m_width(width), m_height(height)
        {
            m_fov = 90;
            m_near = 1;
            m_far = 100;

            m_lookFrom = Vector3{0, 0, 0};
            m_lookAt = Vector3{0, 0, -2};
            m_up = Vector3{0, 1, 0};

            m_focalLen = 1;
        }

        CameraBuilder &setFov(Real fov)
        {
            m_fov = fov;
            return *this;
        }

        CameraBuilder &setNear(Real near)
        {
            m_near = near;
            return *this;
        }

        CameraBuilder &setFar(Real far)
        {
            m_far = far;
            return *this;
        }

        CameraBuilder &setLookFrom(const Vector3 &lookFrom)
        {
            m_lookFrom = lookFrom;
            return *this;
        }

        CameraBuilder &setLookAt(const Vector3 &lookAt)
        {
            m_lookAt = lookAt;
            return *this;
        }

        CameraBuilder &setUp(const Vector3 &up)
        {
            m_up = up;
            return *this;
        }

        CameraBuilder &setFocalLen(Real focalLen)
        {
            m_focalLen = focalLen;
            return *this;
        }

        Camera build()
        {
            cu_utils::Camera camera(m_width, m_height, m_near, m_far, m_fov, m_lookFrom, m_lookAt, m_up, m_focalLen);
            return camera;
        }

    private:
        int m_width;
        int m_height;

        Real m_fov;
        Real m_near;
        Real m_far;

        Vector3 m_lookFrom;
        Vector3 m_lookAt;
        Vector3 m_up;

        Real m_focalLen;
    };
}