#ifndef QUAT_H
#define QUAT_H

#include "Vec3.h"

// Simple Quaternion class.
class Quat {
public:
    // q = xi + yj + zk + w.
    float x, y, z, w;

    // Identity rotation.
    Quat()
        : x(0), y(0), z(0), w(1) {
        // Nothing.
    }

    Quat(float x_, float y_, float z_, float w_)
        : x(x_), y(y_), z(z_), w(w_) {

        // Nothing.
    }

    Quat(const Vec3 &p, float w_)
        : x(p.x()), y(p.y()), z(p.z()), w(w_) {

        // Nothing.
    }

    Quat inv() const {
        return conj()/squared_length();
    }

    Vec3 vec3() const {
        return Vec3(x, y, z);
    }

    Quat conj() const {
        return Quat(-x, -y, -z, w);
    }

    float squared_length() const {
        return x*x + y*y + z*z + w*w;
    }

    Quat operator*(const Quat &o) const {
        return Quat(
                w*o.x + o.w*x + y*o.z - z*o.y,
                w*o.y + o.w*y + z*o.x - x*o.z,
                w*o.z + o.w*z + x*o.y - y*o.x,
                w*o.w - x*o.x - y*o.y - z*o.z);
    }

    Quat operator*(const Vec3 &p) const {
        return Quat(
                w*p.x() + y*p.z() - z*p.y(),
                w*p.y() + z*p.x() - x*p.z(),
                w*p.z() + x*p.y() - y*p.x(),
                -x*p.x() - y*p.y() - z*p.z());
    }

    Quat operator/(float s) const {
        return Quat(x/s, y/s, z/s, w/s);
    }
};

#endif // QUAT_H
