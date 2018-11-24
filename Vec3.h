#ifndef VEC3_H
#define VEC3_H

#include <math.h>
#include <stdlib.h>
#include <iostream>

/**
 * Vector class with all the usual operators.
 */
class Vec3 {
public:
    float e[3];

    Vec3() {
        // Nothing.
    }
    Vec3(float e0, float e1, float e2) {
        e[0] = e0;
        e[1] = e1;
        e[2] = e2;
    }
    inline float x() const { return e[0]; }
    inline float y() const { return e[1]; }
    inline float z() const { return e[2]; }
    inline float r() const { return e[0]; }
    inline float g() const { return e[1]; }
    inline float b() const { return e[2]; }

    inline const Vec3 &operator+() const { return *this; }
    inline Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }
    inline float operator[](int i) const { return e[i]; }
    inline float &operator[](int i) { return e[i]; }

    inline Vec3 operator+(const Vec3 &o) const {
        return Vec3(e[0] + o.e[0], e[1] + o.e[1], e[2] + o.e[2]);
    }
    inline Vec3 operator-(const Vec3 &o) const {
        return Vec3(e[0] - o.e[0], e[1] - o.e[1], e[2] - o.e[2]);
    }
    inline Vec3 operator*(const Vec3 &o) const {
        return Vec3(e[0]*o.e[0], e[1]*o.e[1], e[2]*o.e[2]);
    }
    inline Vec3 operator/(const Vec3 &o) const {
        return Vec3(e[0]/o.e[0], e[1]/o.e[1], e[2]/o.e[2]);
    }
    inline Vec3 operator*(float t) const {
        return Vec3(e[0]*t, e[1]*t, e[2]*t);
    }
    inline Vec3 operator/(float t) const {
        return Vec3(e[0]/t, e[1]/t, e[2]/t);
    }
    inline Vec3 &operator+=(const Vec3 &o) {
        e[0] += o.e[0];
        e[1] += o.e[1];
        e[2] += o.e[2];
        return *this;
    }
    inline Vec3 &operator-=(const Vec3 &o) {
        e[0] -= o.e[0];
        e[1] -= o.e[1];
        e[2] -= o.e[2];
        return *this;
    }
    inline Vec3 &operator*=(const Vec3 &o) {
        e[0] *= o.e[0];
        e[1] *= o.e[1];
        e[2] *= o.e[2];
        return *this;
    }
    inline Vec3 &operator/=(const Vec3 &o) {
        e[0] /= o.e[0];
        e[1] /= o.e[1];
        e[2] /= o.e[2];
        return *this;
    }
    inline Vec3 &operator*=(float t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }
    inline Vec3 &operator/=(float t) {
        float recip = 1/t;
        e[0] *= recip;
        e[1] *= recip;
        e[2] *= recip;
        return *this;
    }
    inline Vec3 unit() const {
        return *this/length();
    }
    inline float dot(const Vec3 &o) const {
        return e[0]*o.e[0] + e[1]*o.e[1] + e[2]*o.e[2];
    }

    inline Vec3 cross(const Vec3 &o) {
        return Vec3(
                e[1]*o.e[2] - e[2]*o.e[1],
                e[2]*o.e[0] - e[0]*o.e[2],
                e[0]*o.e[1] - e[1]*o.e[0]);
    }

    inline float length() const {
        return sqrt(squared_length());
    }
    inline float squared_length() const {
        return dot(*this);
    }
    inline void make_unit_vector() {
        *this /= length();
    }
    inline Vec3 min(const Vec3 &o) const {
        return Vec3(
                fmin(e[0], o.e[0]),
                fmin(e[1], o.e[1]),
                fmin(e[2], o.e[2]));
    }
    inline Vec3 max(const Vec3 &o) const {
        return Vec3(
                fmax(e[0], o.e[0]),
                fmax(e[1], o.e[1]),
                fmax(e[2], o.e[2]));
    }
};

inline Vec3 operator*(float t, const Vec3 &v) {
    return v*t;
}
inline std::istream &operator>>(std::istream &is, Vec3 &v) {
    is >> v.e[0] >> v.e[1] >> v.e[2];
    return is;
}
inline std::ostream &operator<<(std::ostream &os, const Vec3 &v) {
    os << v.e[0] << " " << v.e[1] << " " << v.e[2];
    return os;
}
// v and n must be normalized.
inline Vec3 reflect(const Vec3 &v, const Vec3 &n) {
    return v - 2*v.dot(n)*n;
}
// v and n must be normalized. Returns whether refraction was possible.
bool refract(const Vec3 &v, const Vec3 &n, float ni_over_nt, Vec3 &refracted);

Vec3 random_in_unit_sphere();
// In XY plane.
Vec3 random_in_unit_disc();

// Convert normalized point on sphere to polar coordinates.
void vector_to_polar(const Vec3 &p, float &u, float &v);

// Thread-safe version of drand48(). Returns [0,1).
float my_rand();
void init_rand(int seed);

// Color functions.
Vec3 hsv2rgb(const Vec3 &hsv);

static Vec3 VEC3_BLACK = Vec3(0, 0, 0);
static Vec3 VEC3_ORIGIN = Vec3(0, 0, 0);
static Vec3 VEC3_ONES = Vec3(1, 1, 1);

#endif // VEC3_H
