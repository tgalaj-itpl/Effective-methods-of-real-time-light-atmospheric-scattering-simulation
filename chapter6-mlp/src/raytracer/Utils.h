#pragma once

#include <cmath>
#include <algorithm>
#include <string>
#include <cstdio>

static bool solveQuadraticEquation(double a, double b, double c, double& x1, double& x2)
{
    if (b == 0)
    {
        if (a == 0)
        {
            return false;
        }

        x1 = 0;
        x2 = std::sqrt(-c / a);

        return true;
    }

    double delta = b * b - 4 * a * c;

    if (delta < 0)
    {
        return false;
    }

    double q = (b < 0.0f) ? -0.5f * (b - std::sqrt(delta)) : -0.5f * (b + std::sqrt(delta));

    x1 = q / a;
    x2 = c / q;

    return true;
}

/**
 * @brief Prints a progress bar in a console.
 *        Call in a loop to create console progress bar.
 *
 * @param iteration   - Required  : current iteration
 * @param total       - Required  : total iterations
 * @param prefix      - Optional  : prefix string
 * @param suffix      - Optional  : suffix string
 * @param decimals    - Optional  : positive number of decimals in percent complete
 * @param length      - Optional  : character length of bar
 * @param fill        - Optional  : bar fill character
 * @param printEnd    - Optional  : end character (e.g. "\r", "\r\n")
 */
static void printProgressBar(int iteration, int total, const std::string& prefix = "", const std::string& suffix = "", int decimals = 1, int length = 25, char fill = 254)
{
    float percent = 100.0 * (iteration / float(total));
    int filled_length = length * iteration / total;
    std::string bar = std::string(filled_length, fill) + std::string(length - filled_length, '-');

    std::printf("\r%s |%s| %.2f%% %s\r", prefix.c_str(), bar.c_str(), percent, suffix.c_str());
}

//
// Simple and fast atof (ascii to float) function.
//
// - Executes about 5x faster than standard MSCRT library atof().
// - An attractive alternative if the number of calls is in the millions.
// - Assumes input is a proper integer, fraction, or scientific format.
// - Matches library atof() to 15 digits (except at extreme exponents).
// - Follows atof() precedent of essentially no error checking.
//
// 09-May-2009 Tom Van Baak (tvb) www.LeapSecond.com
//

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

static double atof_fast(const char *p)
{
    int frac;
    double sign, value, scale;

    // Skip leading white space, if any.

    while (white_space(*p)) {
        p += 1;
    }

    // Get sign, if any.

    sign = 1.0;
    if (*p == '-') {
        sign = -1.0;
        p += 1;

    }
    else if (*p == '+') {
        p += 1;
    }

    // Get digits before decimal point or exponent, if any.

    for (value = 0.0; valid_digit(*p); p += 1) {
        value = value * 10.0 + (*p - '0');
    }

    // Get digits after decimal point, if any.

    if (*p == '.') {
        double pow10 = 10.0;
        p += 1;
        while (valid_digit(*p)) {
            value += (*p - '0') / pow10;
            pow10 *= 10.0;
            p += 1;
        }
    }

    // Handle exponent, if any.

    frac = 0;
    scale = 1.0;
    if ((*p == 'e') || (*p == 'E')) {
        unsigned int expon;

        // Get sign of exponent, if any.

        p += 1;
        if (*p == '-') {
            frac = 1;
            p += 1;

        }
        else if (*p == '+') {
            p += 1;
        }

        // Get digits of exponent, if any.

        for (expon = 0; valid_digit(*p); p += 1) {
            expon = expon * 10 + (*p - '0');
        }
        if (expon > 308) expon = 308;

        // Calculate scaling factor.

        while (expon >= 50) { scale *= 1E50; expon -= 50; }
        while (expon >= 8) { scale *= 1E8;  expon -= 8; }
        while (expon > 0) { scale *= 10.0; expon -= 1; }
    }

    // Return signed and scaled floating point result.

    return sign * (frac ? (value / scale) : (value * scale));
}