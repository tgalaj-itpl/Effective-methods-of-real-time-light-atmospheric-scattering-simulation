#include "Timing.h"
#include <chrono>

double Timing::getTime()
{
    auto now = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() / static_cast<double>(SECOND);
}
