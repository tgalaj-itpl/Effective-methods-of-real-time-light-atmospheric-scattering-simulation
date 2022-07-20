#include "Spline2dSolution.h"

#include <cmath>
#include <functional>

solution::Spline2dPrecomputation::Spline2dPrecomputation(const int heightPoints,
                                                         const int distancePoints,
                                                         const double H0,
                                                         Situation _situation, 
                                                         double planet_radius, 
                                                         double atmosphere_radius, 
                                                         double scaling_factor) : situation(_situation), 
                                                                                  r        (planet_radius), 
                                                                                  R        (atmosphere_radius)
{
    offset = 1000.0 * scaling_factor;
    offset2 = 0.0000001 * scaling_factor;

    double lb, ub, dlb, dub, const_factor;
    std::function<double(const double&, const double&, const double&)> function;
    if(situation == Above)
    {
        lb = 0.0;
        ub = sqrt(R*R - r * r);
        dlb = r - offset;
        dub = R;
        function = [&](const double &t, const double &d, const double &H0) { return exp((-sqrt(t*t + d * d) + r) / H0); };
    }
    else
    {
        lb = 0.0;
        ub = R-r;
        dlb = 0;
        dub = r - offset;
        function = [&](const double &h, const double &d, const double &H0) { return exp(-h / H0) * (h + r) / sqrt(((h + r) + d) * ((h + r) - d)); };
    }
    std::vector<double> x, y, v;

    #if USE_UNEVEN_INTERVALS_DISTANCE_POINTS
    int i = 0;
    const_factor = (dub - dlb) / -log10(1.0 - (distancePoints - 1) / double(distancePoints));
    
    for (auto d = dlb; d <= dub - (dub - dlb) / (distancePoints-1); ++i)
    {
        double q_i = 1.0 - double(i) / double(distancePoints);
        d = -std::log10(q_i) * const_factor + dlb;
        x.push_back(d);
    }
    #else
    for (auto d = dlb; d <= dub - (dub - dlb) / (distancePoints-1);)
    {
        x.push_back(d);
        d = d + (dub - dlb) / (distancePoints - 1);
    }
    x.push_back(dub);
    #endif

    #if USE_UNEVEN_INTERVALS_HEIGHT_POINTS
    int j = 0;
    const_factor = (ub - lb) / -log10(1.0 - (heightPoints - 1) / double(heightPoints));

    for (auto dy = lb; dy <= ub - (ub - lb) / (heightPoints-1); ++j)
    {
        double q_j = 1.0 - double(j) / double(heightPoints);
        dy = -std::log10(q_j) * const_factor + lb;
        y.push_back(dy);
    }
    #else
    for (auto dy = lb; dy <= ub - (ub - lb) / (heightPoints-1);)
    {
        y.push_back(dy);
        dy = dy + (ub - lb) / (heightPoints - 1);
    }
    y.push_back(ub);
    #endif

    for (auto i = 0; i < static_cast<int>(y.size()); i++)
    {
        for (auto j = 0; j < static_cast<int>(x.size()); j++)
        {
            v.push_back(function(y[i], x[j], H0));
        }
    }

    alglib::real_1d_array arrX, arrY, arrV;
    arrX.setcontent(x.size(), x.data());
    arrY.setcontent(y.size(), y.data());
    arrV.setcontent(v.size(), v.data());
    alglib::spline2dinterpolant spline;
    alglib::ae_int_t m(x.size()), n(y.size()), dim(1);
    alglib::spline2dbuildbicubicv(arrX, m, arrY, n, arrV, dim, spline);
    alglib::spline2dunpackv(spline, m, n, dim, coefficients);

    /* Precompute spline1d */
    for (auto d = dlb; d <= dub - (dub - dlb) / (distancePoints-1);)
    {
        m_splines1d[d] = spline2dToSpline1d(d);
        d = d + (dub - dlb) / (distancePoints - 1);
    }
    m_splines1d[dub - offset2] = spline2dToSpline1d(dub - offset2);
}

alglib::spline1dinterpolant solution::Spline2dPrecomputation::spline2dToSpline1d(const double distance) const
{
    alglib::spline1dinterpolant result;
    std::vector<double> x, y, d;
    for (auto i = 0; i < coefficients.rows(); i++)
    {
        if (distance < coefficients[i][0] || distance >= coefficients[i][1]) continue;
        const auto dx = distance - coefficients[i][0];
        const auto xI = coefficients[i][2];
        const auto xII = coefficients[i][3];
        const auto C0 = coefficients[i][4] + (coefficients[i][8]  + (coefficients[i][12] + coefficients[i][16] * dx) * dx) * dx;
        const auto C1 = coefficients[i][5] + (coefficients[i][9]  + (coefficients[i][13] + coefficients[i][17] * dx) * dx) * dx;
        const auto C2 = coefficients[i][6] + (coefficients[i][10] + (coefficients[i][14] + coefficients[i][18] * dx) * dx) * dx;
        const auto C3 = coefficients[i][7] + (coefficients[i][11] + (coefficients[i][15] + coefficients[i][19] * dx) * dx) * dx;
        const auto t = xII - xI;
        if (x.empty())
        {
            x.push_back(xI);
            y.push_back(C0);
            d.push_back(C1);
        }

        x.push_back(xII);
        y.push_back(C0 + (C1 + (C2 + C3 * t) * t) * t);
        d.push_back(C1 + (2 * C2 + 3 * C3 * t) * t);
    }

    alglib::real_1d_array arrX, arrY, arrD;
    arrX.setcontent(x.size(), x.data());
    arrY.setcontent(y.size(), y.data());
    arrD.setcontent(d.size(), d.data());
    alglib::spline1dbuildhermite(arrX, arrY, arrD, result);
    return result;
}

double solution::Spline2dPrecomputation::integralValue(const alglib::spline1dinterpolant &spline, const double h1, const double h2, const double d) const
{
    double x1 = 0.0, x2 = 0.0;
    if (situation == Above)
    {
        //if (h1 < d - r || h1 > R - r || h2 < d - r || h2 > R - r)
        //{
        //    // throw std::runtime_error("Wrong heights for this distance!");
        //    std::cerr << "#Above# Wrong heights for this distance!" << std::endl;
        //}
        x1 = std::sqrt((h1 + r) * (h1 + r) - d * d);
        x2 = std::sqrt((h2 + r) * (h2 + r) - d * d);
    }
    else
    {
        //if (h1 < 0.0 || h1 > R - r || h2 < 0.0 || h2 > R - r)
        //{
        //    // throw std::runtime_error("Wrong heights for this distance!");
        //    std::cerr << "#Below# Wrong heights for this distance!" << std::endl;
        //}
        x1 = h1;
        x2 = h2;
    }
    return alglib::spline1dintegrate(spline, x2) - alglib::spline1dintegrate(spline, x1);
}

#define SMOOTHERSTEP(x) ((x) * (x) * (x) * ((x) * ((x) * 6.0 - 15.0) + 10.0))

double solution::Spline2dPrecomputation::integralValue(const double h1, const double h2, const double d)
{
    /* Find correct splines */
    alglib::spline1dinterpolant spline_low, spline_up;
    
    auto spline_low_it = m_splines1d.lower_bound(d);
    auto spline_up_it = spline_low_it;
 
    if(spline_low_it == m_splines1d.end())
    {
		printf("No spline found for d = %.2f", d);
		return 0.0;
    }

    if(spline_low_it == m_splines1d.begin())
    {
        spline_up_it = std::next(spline_low_it);
    }
    else
    {
        spline_up_it = std::prev(spline_low_it);
    }

    if(spline_low_it->first > spline_up_it->first)
    {
        std::swap(spline_low_it, spline_up_it);
    }

    spline_low = spline_low_it->second;
    spline_up  = spline_up_it->second;

	double x11 = 0.0, x2 = 0.0;
	if (situation == Above)
	{
		//if (h1 < d - r || h1 > R - r || h2 < d - r || h2 > R - r)
		//{
		//    // throw std::runtime_error("Wrong heights for this distance!");
		//    std::cerr << "#Above# Wrong heights for this distance!" << std::endl;
		//}
		x11 = std::sqrt((h1 + r) * (h1 + r) - d * d);
		x2 = std::sqrt((h2 + r) * (h2 + r) - d * d);
	}
	else
	{
		//if (h1 < 0.0 || h1 > R - r || h2 < 0.0 || h2 > R - r)
		//{
		//    // throw std::runtime_error("Wrong heights for this distance!");
		//    std::cerr << "#Below# Wrong heights for this distance!" << std::endl;
		//}
		x11 = h1;
		x2 = h2;
	}

    double int_low = alglib::spline1dintegrate(spline_low, x2) - alglib::spline1dintegrate(spline_low, x11);
    double int_up  = alglib::spline1dintegrate(spline_up , x2) - alglib::spline1dintegrate(spline_up , x11);

    double x0 = spline_low_it->first;
    double x1 = spline_up_it->first;

    double y0 = int_low;
    double y1 = int_up;

    /* linear interpolation */
    return y0 + ((d - x0) / (x1 - x0) * (y1 - y0));
    
    /* Smoothstep */
    //double t = glm::smoothstep(x0, x1, d);
    //return glm::mix(y0, y1, t);
}