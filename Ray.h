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

    Ray() {
        // Nothing.
    }
    Ray(const Vec3 &origin, const Vec3 &direction)
        : m_origin(origin), m_direction(direction) {

        // Nothing.
    }
    const Vec3 &origin() const { return m_origin; }
    const Vec3 &direction() const { return m_direction; }
    Vec3 point_at(float t) const { return m_origin + t*m_direction; }
};

#endif // RAY_H
