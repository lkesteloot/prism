
// Scene config:
// X is to the right, Y is up, and Z is towards the viewer.
// Center of image is at 0,0.
// Ground (paper) is at Z = 0.
// Width of image is 1 (from -0.5 to 0.5).
// Height of image depends on output image file size.
// Prism is 2 units high (in positive Z direction).
// Prism is 0.5 units wide at its base and 0.87 units high.
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
static const int HEIGHT = 4200/10;
static const int BYTES_PER_PIXEL = 4;
static const int STRIDE = WIDTH*BYTES_PER_PIXEL;
static const int BYTE_COUNT = STRIDE*HEIGHT;
static const int PIXEL_COUNT = WIDTH*HEIGHT;

// Whether to quit the program.
static bool g_quit;

// Number of threads to use.
static int g_thread_count;

// How many worker threads are still working.
static std::atomic_int g_working;

void render_image(unsigned char *image, int seed) {
    // Initialize the seed for our thread.
    init_rand(seed);

    while (!g_quit) {
        // Random ray from light source, through slit.
        Vec3 ray_origin = Vec3(-10, -10, 1);
        Vec3 ray_target = Vec3(-0.6, my_rand()*0.1, my_rand());
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
                int i = y*STRIDE + x*BYTES_PER_PIXEL;
                image[i] = std::min(image[i] + 1, 0xFF);
                i++;
                image[i] = std::min(image[i] + 1, 0xFF);
                i++;
                image[i] = std::min(image[i] + 1, 0xFF);
                i++;
                image[i++] = 0xFF;
            }
        }
    }

    // We're no longer working.
    g_working--;
}

void render_frame() {
    unsigned char *image = new unsigned char[BYTE_COUNT];

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
        int state = mfb_update(image);
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

    // Save the image if we weren't interrupted by the user.
    if (!g_quit) {
        // Convert from RGBA to RGB.
        const int RGB_BYTE_COUNT = PIXEL_COUNT*3;
        unsigned char *rgb_image = new unsigned char[RGB_BYTE_COUNT];
        unsigned char *rgba = image;
        unsigned char *rgb = rgb_image;
        for (int i = 0; i < PIXEL_COUNT; i++) {
            rgb[0] = rgba[2];
            rgb[1] = rgba[1];
            rgb[2] = rgba[0];

            rgba += 4;
            rgb += 3;
        }

        // Write image.
        int success = stbi_write_png("out.png", WIDTH, HEIGHT, 3, rgb_image, WIDTH*3);
        if (!success) {
            std::cerr << "Cannot write output image.\n";
        }
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
