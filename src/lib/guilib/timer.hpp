
#pragma once

#include <chrono>

class Timer
{
public:
    void start();
    void stop();
    double elapsedMilliseconds();
    double elapsedSeconds();

private:
    typedef std::chrono::time_point<std::chrono::system_clock> chrono_time_t;
    chrono_time_t m_StartTime;
    chrono_time_t m_EndTime;
    bool m_bRunning = false;
};