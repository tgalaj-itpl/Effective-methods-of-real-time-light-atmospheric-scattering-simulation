#pragma once
#include "alglib-3.19.0/src/interpolation.h"
#include <map>

#define USE_UNEVEN_INTERVALS_DISTANCE_POINTS 0
#define USE_UNEVEN_INTERVALS_HEIGHT_POINTS 0

namespace solution
{
    class Spline2dPrecomputation
    {
    public:
        enum Situation
        {
            Above,
            Below
        };
        Spline2dPrecomputation(const int heightPoints, const int distancePoints, const double H0, Situation _situation, double planet_radius, double atmosphere_radius, double scaling_factor);
        double integralValue(const alglib::spline1dinterpolant &spline, const double h1, const double h2, const double d) const;
        double integralValue(const double h1, const double h2, const double d);
        alglib::spline1dinterpolant spline2dToSpline1d(const double distance) const;

        double offset;
        double offset2;

    private:
        Situation situation;
        alglib::real_2d_array coefficients;
        double r, R;

        std::map<double, alglib::spline1dinterpolant> m_splines1d;
    };
} // namespace solution