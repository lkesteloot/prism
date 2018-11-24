#ifndef TIMER_H
#define TIMER_H

#include <chrono>

using namespace std::chrono;

// Records elapsed time since object instantiation.
class Timer {
    steady_clock::time_point m_start_time;

public:
    Timer() {
        m_start_time = steady_clock::now();
    }

    // Returns elapsed time in seconds since object creation.
    double elapsed() const {
        steady_clock::time_point now = steady_clock::now();

        steady_clock::duration time_span = now - m_start_time;

        return double(time_span.count())*steady_clock::period::num/steady_clock::period::den;
    }
};

#endif // TIMER_H
