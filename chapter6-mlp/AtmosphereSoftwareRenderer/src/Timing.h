#pragma once
class Time
{
public:
    Time();
    ~Time();

    static double getTime();

private:
    static const long long SECOND = 1000000000L;

    static double delta;
};

