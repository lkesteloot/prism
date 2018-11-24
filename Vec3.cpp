
#include "Vec3.h"

static Vec3 VEC3_XY_ONES = Vec3(1, 1, 0);

// Thread-local state for our random number generator.
thread_local unsigned short g_xsubi[3];

void init_rand(int seed) {
    g_xsubi[0] = seed;
    g_xsubi[1] = seed;
    g_xsubi[2] = seed;
}

float my_rand() {
    return erand48(g_xsubi);
}

Vec3 random_in_unit_sphere() {
    Vec3 p;

    do {
        p = 2.0*Vec3(my_rand(), my_rand(), my_rand()) - VEC3_ONES;
    } while (p.squared_length() > 1.0);

    return p;
}

Vec3 random_in_unit_disc() {
    Vec3 p;

    do {
        p = 2.0*Vec3(my_rand(), my_rand(), 0) - VEC3_XY_ONES;
    } while (p.squared_length() > 1.0);

    return p;
}

void vector_to_polar(const Vec3 &p, float &u, float &v) {
    float phi = atan2(p.z(), p.x());
    float theta = asin(p.y());

    u = 1 - (phi + M_PI) / (2*M_PI);
    v = (theta + M_PI/2) / M_PI;
}

bool refract(const Vec3 &v, const Vec3 &n, float ni_over_nt, Vec3 &refracted) {
    float dt = v.dot(n);
    float discriminant = 1 - ni_over_nt*ni_over_nt*(1 - dt*dt);
    if (discriminant > 0) {
        refracted = ni_over_nt*(v - n*dt) - n*sqrt(discriminant);
        return true;
    }

    // Total internal reflection.
    return false;
}

// https://stackoverflow.com/a/7901693/211234
Vec3 hsv2rgb(const Vec3 &hsv) {
    float h = hsv.e[0];
    float s = hsv.e[1];
    float v = hsv.e[2];

    // Unsaturated is just gray.
    if (s <= 0.0) {
        return Vec3(v, v, v);
    }

    // Wrap hue.
    h = fmodf(h, 1);

    // Find section.
    h *= 6;
    int section = (int) h;

    // Find hue within section.
    h -= section;

    float p = v*(1 - s);
    float q = v*(1 - s*h);
    float t = v*(1 - s*(1.0 - h));

    switch(section) {
        case 0:
            return Vec3(v, t, p);
            break;

        case 1:
            return Vec3(q, v, p);
            break;

        case 2:
            return Vec3(p, v, t);
            break;

        case 3:
            return Vec3(p, q, v);
            break;

        case 4:
            return Vec3(t, p, v);
            break;

        case 5:
        default:
            return Vec3(v, p, q);
            break;
    }
}
