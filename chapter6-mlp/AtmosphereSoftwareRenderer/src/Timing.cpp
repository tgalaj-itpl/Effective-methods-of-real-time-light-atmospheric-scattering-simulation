#include <chrono>
#include "Timing.h"

double Time::delta = 0.0f;

Time::Time()
{
}


Time::~Time()
{
}

double Time::getTime()
{
    auto now = std::chrono::system_clock::now();
    return double(std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count())/double(SECOND);
}
