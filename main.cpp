
// Scene config:
// X is to the right, Y is up, and Z is towards the viewer.
// Center of image is at 0,0.
// Ground (paper) is at Z = 0.
// Width of image is 1 (from -0.5 to 0.5).
// Height of image depends on output image file size.
// Prism is 2 units high (in positive Z direction).
// Prism is 0.3 units wide at its base.
// Prism's base is centered at 0,0.
// Single light source at -10, -10, 1.

#include <iostream>
#include <float.h>
#include <thread>
#include <vector>
#include <unistd.h>
#include "Ray.h"

#ifdef DISPLAY
#include "MiniFB.h"
#endif

#include "stb_image_write.h"

static const int WIDTH = 3300/10;
static const int HEIGHT = 3300/10; // 4200
static const int PIXEL_COUNT = WIDTH*HEIGHT;
static const float PRISM_WIDTH = 0.3;

// Whether to quit the program.
static bool g_quit;

// Number of threads to use.
static int g_thread_count;

// How many worker threads are still working.
static std::atomic_int g_working;

/**
 * Given a wavelength in nanometers, returns RGB value between 0 and 1.
 *
 * From: https://www.johndcook.com/wavelength_to_RGB.html
 */
void wavelength_to_rgb(int wavelength, float rgb[3]) {
    float red, green, blue;

    if (wavelength >= 380 && wavelength < 440) {
        red   = -(wavelength - 440) / (440 - 380.);
        green = 0.0;
        blue  = 1.0;
    } else if (wavelength >= 440 && wavelength < 490) {
        red   = 0.0;
        green = (wavelength - 440) / (490 - 440.);
        blue  = 1.0;
    } else if (wavelength >= 490 && wavelength < 510) {
        red   = 0.0;
        green = 1.0;
        blue  = -(wavelength - 510) / (510 - 490.);
    } else if (wavelength >= 510 && wavelength < 580) {
        red   = (wavelength - 510) / (580 - 510.);
        green = 1.0;
        blue  = 0.0;
    } else if (wavelength >= 580 && wavelength < 645) {
        red   = 1.0;
        green = -(wavelength - 645) / (645 - 580.);
        blue  = 0.0;
    } else if (wavelength >= 645 && wavelength < 781) {
        red   = 1.0;
        green = 0.0;
        blue  = 0.0;
    } else {
        red   = 0.0;
        green = 0.0;
        blue  = 0.0;
    }

    // Let the intensity fall off near the vision limits.
    float factor;
    if (wavelength >= 380 && wavelength < 420) {
        factor = 0.3 + 0.7*(wavelength - 380) / (420 - 380.);
    } else if (wavelength >= 420 && wavelength < 701) {
        factor = 1.0;
    } else if (wavelength >= 701 && wavelength < 781) {
        factor = 0.3 + 0.7*(780 - wavelength) / (780 - 700.);
    } else {
        factor = 0.0;
    }

    rgb[0] = red*factor;
    rgb[1] = green*factor;
    rgb[2] = blue*factor;
}

// Normalized 2D normal vector to two vertices.
Vec3 get_2d_normal(Vec3 const &p1, Vec3 const &p2) {
    Vec3 v = p2 - p1;

    return Vec3(-v.y(), v.x(), 0).unit();
}

float intersect_with_prism_side(Ray const &ray,
        Vec3 const &p1, Vec3 const &p2, Vec3 const &n) {

    Vec3 p = ray.origin() - p1;

    float denom = ray.direction().dot(n);
    if (denom == 0) {
        return -1;
    }

    float t = -(p.dot(n)) / denom;

    return t;
}

void plot_point(float *image, Vec3 const &p) {
    Vec3 pi = (p + Vec3(0.5, 0.5, 0))*WIDTH;

    int x = (int) (pi.x() + 0.5);
    int y = HEIGHT - 1 - (int) (pi.y() + 0.5);

    // std::cout << x << ", " << y << "\n";
    if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT) {
        int i = (y*WIDTH + x)*3;
        image[i + 0] = 1;
        image[i + 1] = 1;
        image[i + 2] = 1;
    }
}

void render_image(float *image, int seed) {
    // Initialize the seed for our thread.
    init_rand(seed);

    // 2D vertices of prism, clockwise from lower-left.
    Vec3 p0(-PRISM_WIDTH/2, 0, 0);
    Vec3 p1(0, PRISM_WIDTH*sqrt(3)/2, 0);
    Vec3 p2(PRISM_WIDTH/2, 0, 0);

    // 2D normals of prism, clockwise from left face.
    Vec3 n01 = get_2d_normal(p0, p1);
    Vec3 n12 = get_2d_normal(p1, p2);
    Vec3 n20 = get_2d_normal(p2, p0);

    plot_point(image, p0);
    plot_point(image, p1);
    plot_point(image, p2);

    while (!g_quit) {
        // Random ray from light source, through slit.
        Vec3 ray_origin = Vec3(-10, -2, 1);
        Vec3 ray_target = Vec3(-0.6, my_rand()*0.01, my_rand());
        int wavelength = (int) (380 + (700 - 380)*my_rand());
        Ray ray(ray_origin, ray_target - ray_origin, wavelength);

        // Intersect with prism.
        bool hit_prism = false;
        float t = intersect_with_prism_side(ray, p0, p1, n01);
        if (t >= 0) {
            Vec3 p = ray.point_at(t);
            plot_point(image, p);
            if (p.z() > 0) {
                hit_prism = true;
            }
        }

        // Intersect with ground plane.
        float dz = ray.m_direction.z();
        if (!hit_prism && dz != 0) {
            float t = -ray.m_origin.z()/dz;

            Vec3 p = ray.point_at(t);
            p = (p + Vec3(0.5, 0.5, 0))*WIDTH;

            int x = (int) (p.x() + 0.5);
            int y = HEIGHT - 1 - (int) (p.y() + 0.5);

            if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT) {
                float rgb[3];
                wavelength_to_rgb(ray.wavelength(), rgb);

                int i = (y*WIDTH + x)*3;
                image[i + 0] += rgb[0]*0.001;
                image[i + 1] += rgb[1]*0.001;
                image[i + 2] += rgb[2]*0.001;
            }
        }
    }

    // We're no longer working.
    g_working--;
}

void render_frame() {
    float *image = new float[PIXEL_COUNT*3];

#ifdef DISPLAY
    // For display.
    uint32_t *image32 = new uint32_t[PIXEL_COUNT];
#endif

    g_quit = false;

    // g_thread_count = std::thread::hardware_concurrency();
    g_thread_count = 1;
    std::cout << "Using " << g_thread_count << " threads.\n";

    g_working = g_thread_count;

    // Generate the image on multiple threads.
    std::vector<std::thread *> thread;
    for (int t = 0; t < g_thread_count; t++) {
        thread.push_back(new std::thread(render_image, image, random()));
    }

#ifdef DISPLAY
    while (g_working > 0) {
        // Convert from float to 32-bit integer.
        float *rgbf = image;
        for (int i = 0; i < PIXEL_COUNT; i++) {
            float gamma = 0.80;
            // Avoid negative base.
            float red = rgbf[0] > 0 ? 255*pow(rgbf[0], gamma) : 0;
            float green = rgbf[1] > 0 ? 255*pow(rgbf[1], gamma) : 0;
            float blue = rgbf[2] > 0 ? 255*pow(rgbf[2], gamma) : 0; 

            image32[i] = MFB_RGB(
                    (unsigned char) red,
                    (unsigned char) green,
                    (unsigned char) blue);

            rgbf += 3;
        }

        int state = mfb_update(image32);
        if (state < 0) {
            // Tell workers to quit.
            g_quit = true;
        } else {
            usleep(30*1000);
        }
    }
#endif

    // Wait for worker threads to quit.
    for (int t = 0; t < g_thread_count; t++) {
        thread[t]->join();
        delete thread[t];
        thread[t] = 0;
    }
}

void usage() {
    std::cerr << "Usage: prism\n";
}

int main() {
#ifdef DISPLAY
    if (!mfb_open("ray", WIDTH, HEIGHT)) {
        std::cerr << "Failed to open the display.\n";
        return 0;
    }
#endif

    render_frame();

    return 0;
}
