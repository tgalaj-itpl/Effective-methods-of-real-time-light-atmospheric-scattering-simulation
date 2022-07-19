#pragma once

class Timing
{
public:

    /** @brief Get current time 
      * @result time in seconds
     **/
    static double getTime();

    static const long long SECOND = 1'000'000'000;
};
