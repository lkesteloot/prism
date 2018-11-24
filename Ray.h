#ifndef RAY_H
#define RAY_H

#include "Vec3.h"

/**
 * A ray with an origin and direction. The direction is not
 * necessarily of unit length.
 */
class Ray {
public:
    Vec3 m_origin;
    Vec3 m_direction;
    int m_wavelength;

    Ray() {
        // Nothing.
    }
    Ray(const Vec3 &origin, const Vec3 &direction, int wavelength)
        : m_origin(origin), m_direction(direction), m_wavelength(wavelength) {

        // Nothing.
    }
    const Vec3 &origin() const { return m_origin; }
    const Vec3 &direction() const { return m_direction; }
    int wavelength() const { return m_wavelength; }
    Vec3 point_at(float t) const { return m_origin + t*m_direction; }
};

inline std::ostream &operator<<(std::ostream &os, const Ray &ray) {
    os << "(" << ray.origin() << ")-(" << ray.direction() << ")";
    return os;
}

#endif // RAY_H
