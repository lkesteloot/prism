
// Scene config:
// X is to the right, Y is up, and Z is towards the viewer.
// Center of image is at 0,0.
// Ground (paper) is at Z = 0.
// Width of image is 1 (from -0.5 to 0.5).
// Height of image depends on output image file size.
// Prism is 2 units high (in positive Z direction).
// Prism is 0.3 units wide at its base.

#include <iostream>
#include <sstream>
#include <iomanip>
#include <float.h>
#include <thread>
#include <vector>
#include <limits>
#include <chrono>
#include <atomic>
#include <unistd.h>
#include "Ray.h"

#ifdef DISPLAY
#include "MiniFB.h"
#endif

#include "stb_image_write.h"

// Define this to have a UI pop up with the image in progress (Mac only).
#undef UPDATE_DISPLAY

// Size of the output image. Divide by 5 for in-progress work.
static const int WIDTH = 3300;
static const int HEIGHT = 4200;
static const int PIXEL_COUNT = WIDTH*HEIGHT;
static const float PRISM_WIDTH = 0.3;
static const float MIN_HIT_DIST = 0.001;
static const float GAMMA = 1/2.2;

// How much to zoom into the center of the image (to make the prism look larger).
static const float ZOOM = 2;

// Whether to quit the program.
static bool g_quit;

// Number of threads to use.
static int g_thread_count;

// How many worker threads are still working.
static std::atomic_int g_working;

// Normalized 2D normal vector to two vertices.
Vec3 get_2d_normal(Vec3 const &p1, Vec3 const &p2) {
    Vec3 v = p2 - p1;

    return Vec3(-v.y(), v.x(), 0).unit();
}

// Return the distance along the ray to hit this side of the prism.
float intersect_with_prism_side(Ray const &ray,
        Vec3 const &p1, Vec3 const &p2, Vec3 const &n) {

    Vec3 p = ray.origin() - p1;

    // See if we're parallel to the side.
    float denom = ray.direction().dot(n);
    if (denom == 0) {
        return -1;
    }

    // Distance to intersection.
    float t = -(p.dot(n)) / denom;

    if (t > MIN_HIT_DIST) {
        // See if we're within the rectangle.
        p = ray.point_at(t);

        // Prism is two high.
        if (p.z() > 2) {
            return -1;
        }

        if (fabs(n.x()) > fabs(n.y())) {
            // Vertical side. Project to Y axis.
            if (p1.y() < p2.y()) {
                if (p.y() < p1.y() || p.y() > p2.y()) {
                    return -1;
                }
            } else {
                if (p.y() < p2.y() || p.y() > p1.y()) {
                    return -1;
                }
            }
        } else {
            // Horizontal side. Project to X axis.
            if (p1.x() < p2.x()) {
                if (p.x() < p1.x() || p.x() > p2.x()) {
                    return -1;
                }
            } else {
                if (p.x() < p2.x() || p.x() > p1.x()) {
                    return -1;
                }
            }
        }
    } else {
        t = -1;
    }

    return t;
}

// Plot a single pixel. For debugging.
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

// Approximate reflection coefficient.
static float schlick(float cosine, float refraction_index) {
    float r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 *= r0;
    return r0 + (1 - r0)*pow(1 - cosine, 5);
}

// Get new ray from an intersection with glass.
void hit_glass(Ray const &ray_in, Vec3 const &p, Vec3 const &n,
        float refraction_index, Ray &ray_out) {

    // Our ray's direction, normalized.
    Vec3 dir = ray_in.direction().unit();

    // We don't know whether we're inside the material or outside.
    // Our hit normal will always point outward. We want a normal
    // that points in the direction we came from.
    Vec3 normal;
    float ni_over_nt;
    float cosine;
    if (dir.dot(n) > 0) {
        // We're inside. Reverse normal.
        normal = -n;
        ni_over_nt = refraction_index;
        cosine = refraction_index*dir.dot(n);
    } else {
        // We're outside. Use normal as-is.
        normal = n;
        ni_over_nt = 1/refraction_index;
        cosine = -dir.dot(n);
    }

    Vec3 refracted;
    if (refract(dir, normal, ni_over_nt, refracted)) {
        // We can refract. Figure out if we should.
        float reflection_probability = schlick(cosine, refraction_index);

        if (my_rand() < reflection_probability) {
            Vec3 reflected = reflect(dir, n);
            ray_out = Ray(p, reflected, ray_in.wavelength());
        } else {
            ray_out = Ray(p, refracted, ray_in.wavelength());
        }
    } else {
        // Can't refract. Only reflect.
        Vec3 reflected = reflect(dir, n);
        ray_out = Ray(p, reflected, ray_in.wavelength());
    }
}

// Render into "image" with the specified random seed.
void render_image(float *image, int seed) {
    // Initialize the seed for our thread.
    init_rand(seed);

    // 2D vertices of prism, clockwise from lower-left.
    Vec3 p0(-PRISM_WIDTH/2, 0, 0);
    Vec3 p1(0, PRISM_WIDTH*sqrt(3)/2, 0);
    Vec3 p2(PRISM_WIDTH/2, 0, 0);

    // Center prism at 0,0,0.
    Vec3 offset(0, -p1.y()*0.4, 0);
    p0 += offset;
    p1 += offset;
    p2 += offset;

    // 2D normals of prism, clockwise from left face.
    Vec3 n01 = get_2d_normal(p0, p1);
    Vec3 n12 = get_2d_normal(p1, p2);
    Vec3 n20 = get_2d_normal(p2, p0);

    // Draw vertices of prism, for debugging.
    /// plot_point(image, p0);
    /// plot_point(image, p1);
    /// plot_point(image, p2);

    while (!g_quit) {
        // Random ray from light source, through slit.
        Vec3 ray_origin(-10, -3.2, 1);
        Vec3 ray_target(-0.6, my_rand()*0.002 - 0.05, my_rand());
        int wavelength = (int) (380 + (700 - 380)*my_rand());
        // ray_target = Vec3(-0.6, (wavelength - 380)/(700 - 380.0) - 0.5, my_rand());

        /*
        float xxx = my_rand()*2*M_PI;
        ray_origin = Vec3(3*cos(xxx), 3*sin(xxx), 1);
        ray_target = Vec3(0, 0, my_rand());

        float yyy = ((p0 + p1 + p2)/3).y();
        ray_origin = Vec3(30*cos(xxx), 30*sin(xxx) + yyy, 1);
        ray_target = Vec3(0, yyy, my_rand());

        float xxx2 = my_rand()*2*M_PI;
        ray_target = Vec3(.5*cos(xxx2), .5*sin(xxx2) + yyy, my_rand());
        */

        ray_origin += offset;
        ray_target += offset;

        // Occasionally send some light from above, to highlight the prism itself.
        if (my_rand() < 0.10) {
            Vec3 p_avg = (p0 + p1 + p2)/3;
            ray_origin = p_avg + Vec3((my_rand() - 0.5)*0.1, (my_rand() - 0.5)*0.1, 10);
            ray_target = p_avg + Vec3(my_rand() - 0.5, my_rand() - 0.5, 0);
        }

        Ray ray(ray_origin, (ray_target - ray_origin).unit(), wavelength);

        bool done_with_ray = false;
        while (!done_with_ray) {
            // Intersect distance.
            float best_t = std::numeric_limits<float>::max();
            Vec3 best_p;
            Vec3 best_n;
            int best_obj = -1; // 0-2 = prism, 3 = floor.

            // Intersect with prism.
            float t = intersect_with_prism_side(ray, p0, p1, n01);
            if (t > MIN_HIT_DIST && t < best_t) {
                Vec3 p = ray.point_at(t);
                if (p.z() > 0) {
                    best_t = t;
                    best_p = p;
                    best_n = n01;
                    best_obj = 0;
                }
            }

            t = intersect_with_prism_side(ray, p1, p2, n12);
            if (t > MIN_HIT_DIST && t < best_t) {
                Vec3 p = ray.point_at(t);
                if (p.z() > 0) {
                    best_t = t;
                    best_p = p;
                    best_n = n12;
                    best_obj = 1;
                }
            }

            t = intersect_with_prism_side(ray, p2, p0, n20);
            if (t > MIN_HIT_DIST && t < best_t) {
                Vec3 p = ray.point_at(t);
                if (p.z() > 0) {
                    best_t = t;
                    best_p = p;
                    best_n = n20;
                    best_obj = 2;
                }
            }

            // Intersect with ground plane.
            float dz = ray.m_direction.z();
            if (dz != 0) {
                t = -ray.m_origin.z()/dz;

                if (t > MIN_HIT_DIST && t < best_t) {
                    Vec3 p = ray.point_at(t);

                    best_t = t;
                    best_p = p;
                    best_n = Vec3(0, 0, 1);
                    best_obj = 3;
                }
            }

            switch (best_obj) {
                case -1:
                    // Didn't intersect anything.
                    done_with_ray = true;
                    break;

                case 0:
                case 1:
                case 2: {
                            // Compute index of refraction for wavelength.
                            // https://en.wikipedia.org/wiki/Cauchy%27s_equation
                            float B = 1.5046;
                            float C = 0.00420;

                            // Widen rainbow, renormalize B.
                            float new_C = C*10;
                            B = B + C/(.540*.540) - new_C/(.540*.540);
                            C = new_C;

                            float wl_um = ray.wavelength()/1000.0;
                            float refraction_index = B + C/(wl_um*wl_um);

                            Ray ray_out;
                            hit_glass(ray, best_p, best_n, refraction_index, ray_out);
                            ray = ray_out;
                            /// std::cout << ray << "\n";
                            break;
                        }

                case 3: {
                            // Landed on paper, leave a spot.
                            Vec3 p = (best_p*ZOOM + Vec3(0.5, HEIGHT/2.0/WIDTH, 0))*WIDTH;

                            int x = (int) (p.x() + 0.5);
                            int y = HEIGHT - 1 - (int) (p.y() + 0.5);

                            if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT) {
                                Vec3 rgb = wavelength2rgb(ray.wavelength());

                                int i = (y*WIDTH + x)*3;
                                image[i + 0] += rgb.r()*0.001;
                                image[i + 1] += rgb.g()*0.001;
                                image[i + 2] += rgb.b()*0.001;
                            }
                            done_with_ray = true;
                            break;
                        }
            }
        }
    }

    // We're no longer working.
    g_working--;
}

// Same normalized image to disk.
void save_image(float *image, int file_counter) {
    // Convert from RGBA to RGB.
    unsigned char *rgb_image = new unsigned char[PIXEL_COUNT*3];
    unsigned char *rgb = rgb_image;
    float *rgbf = image;
    for (int i = 0; i < PIXEL_COUNT; i++) {
        rgb[0] = (unsigned char) rgbf[0];
        rgb[1] = (unsigned char) rgbf[1];
        rgb[2] = (unsigned char) rgbf[2];

        rgbf += 3;
        rgb += 3;
    }

    // Write image.
    std::ostringstream final_pathname;
    final_pathname << "out4" << "-" << std::setfill('0') <<
        std::setw(3) << file_counter << ".png";

    std::cout << "Saving to " << final_pathname.str() << "\n";
    int success = stbi_write_png(final_pathname.str().c_str(),
            WIDTH, HEIGHT, 3, rgb_image, WIDTH*3);
    if (!success) {
        std::cerr << "Cannot write output image.\n";
    }
}

// Render a single frame.
void render_frame() {
#ifdef DISPLAY
    // For display.
    float *image_norm = new float[PIXEL_COUNT*3];
    uint32_t *image32 = new uint32_t[PIXEL_COUNT];
#endif

    g_quit = false;

    g_thread_count = std::thread::hardware_concurrency();
    std::cout << "Using " << g_thread_count << " threads.\n";

    g_working = g_thread_count;

    // Generate the image on multiple threads.
    std::vector<float *> images;
    std::vector<std::thread *> thread;
    for (int t = 0; t < g_thread_count; t++) {
        float *image = new float[PIXEL_COUNT*3];
        images.push_back(image);
        thread.push_back(new std::thread(render_image, image, random()));
    }

#ifdef DISPLAY
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    int file_counter = 1;

    while (g_working > 0) {
        // Take log of color.
        float *rgbt = image_norm;
        float max = 0;
        for (int i = 0; i < PIXEL_COUNT; i++) {
            // Add one because log(1) = 0.
            rgbt[0] = 1;
            rgbt[1] = 1;
            rgbt[2] = 1;

            // Add all images.
            for (int j = 0; j < g_thread_count; j++) {
                rgbt[0] += images[j][i*3 + 0];
                rgbt[1] += images[j][i*3 + 1];
                rgbt[2] += images[j][i*3 + 2];
            }

            rgbt[0] = log(rgbt[0]);
            rgbt[1] = log(rgbt[1]);
            rgbt[2] = log(rgbt[2]);

            max = std::max(std::max(std::max(max, rgbt[0]), rgbt[1]), rgbt[2]);

            rgbt += 3;
        }

        // Convert from float to 32-bit integer.
        float *rgbf = image_norm;
        for (int i = 0; i < PIXEL_COUNT; i++) {
            // Avoid negative base.
            rgbf[0] = rgbf[0] > 0 ? 255*pow(rgbf[0]/max, GAMMA) : 0;
            rgbf[1] = rgbf[1] > 0 ? 255*pow(rgbf[1]/max, GAMMA) : 0;
            rgbf[2] = rgbf[2] > 0 ? 255*pow(rgbf[2]/max, GAMMA) : 0; 

            image32[i] = MFB_RGB(
                    (unsigned char) rgbf[0],
                    (unsigned char) rgbf[1],
                    (unsigned char) rgbf[2]);

            rgbf += 3;
        }

#ifdef UPDATE_DISPLAY
        int state = mfb_update(image32);
#else
        int state = 0;
#endif
        if (state < 0) {
            // Tell workers to quit.
            g_quit = true;
        } else {
#ifdef UPDATE_DISPLAY
            usleep(300*1000);
#else
            usleep(300*1000*60);
#endif

            // Periodically save an image.
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            std::chrono::steady_clock::duration time_span = now - start_time;
            double seconds = double(time_span.count())*std::chrono::steady_clock::period::num/
                std::chrono::steady_clock::period::den;
            if (seconds > 60) {
                save_image(image_norm, file_counter++);
                start_time = std::chrono::steady_clock::now();
            }
        }
    }
#endif

    // Wait for worker threads to quit.
    for (int t = 0; t < g_thread_count; t++) {
        thread[t]->join();
        delete thread[t];
        thread[t] = nullptr;

        delete images[t];
        images[t] = nullptr;
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
