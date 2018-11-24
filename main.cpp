
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

// Whether to quit the program.
static bool g_quit;

// Number of threads to use.
static int g_thread_count;

// How many worker threads are still working.
static std::atomic_int g_working;

void render_image(float *image, int seed) {
    // Initialize the seed for our thread.
    init_rand(seed);

    while (!g_quit) {
        // Random ray from light source, through slit.
        Vec3 ray_origin = Vec3(-10, -2, 1);
        Vec3 ray_target = Vec3(-0.6, my_rand()*0.01, my_rand());
        Ray ray(ray_origin, ray_target - ray_origin);

        // Intersect with ground plane.
        float dz = ray.m_direction.z();
        if (dz != 0) {
            float t = -ray.m_origin.z()/dz;

            Vec3 p = ray.point_at(t);
            p = (p + Vec3(0.5, 0.5, 0))*WIDTH;

            int x = (int) (p.x() + 0.5);
            int y = HEIGHT - 1 - (int) (p.y() + 0.5);

            if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT) {
                int i = (y*WIDTH + x)*3;
                image[i + 0] += 0.10;
                image[i + 1] += 0.10;
                image[i + 2] += 0.00;
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
            image32[i] = MFB_RGB(
                    (unsigned char) rgbf[0],
                    (unsigned char) rgbf[1],
                    (unsigned char) rgbf[2]);

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

int main(int argc, char *argv[]) {
#ifdef DISPLAY
    if (!mfb_open("ray", WIDTH, HEIGHT)) {
        std::cerr << "Failed to open the display.\n";
        return 0;
    }
#endif

    render_frame();

    return 0;
}
