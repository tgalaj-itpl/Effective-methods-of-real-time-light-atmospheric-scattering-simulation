#include "Midpoint.h"

Midpoint::Midpoint(const Options & options)
    : Atmosphere(options)
{
    samples       = options.MIDPOINT_SAMPLES;
    samples_light = options.MIDPOINT_SAMPLES_LIGHT;

    std::cout << "MIDPOINT RULE INFO" << std::endl;
    std::cout << "VIEW SAMPLES  = " << samples << std::endl;
    std::cout << "LIGHT SAMPLES = " << samples_light << std::endl << std::endl;
}

std::vector<IntegrationData> Midpoint::integrator(Ray ray, double a, double b, unsigned n, bool precomptute)
{
    std::vector<IntegrationData> data(1);

    double step = (b - a) / double(n);

    if (precomptute)
    {
        data.resize(n);

        for (unsigned i = 0; i < n; ++i)
        {
            data[i].sample_position = ray.m_origin + ray.m_direction * (a + step * (i + 0.5));
            double sample_height    = sampleHeight(data[i].sample_position);

            data[i].rayleigh = glm::exp(-sample_height / h_rayleigh) * step;
            data[i].mie      = glm::exp(-sample_height / h_mie)      * step;
        }
    }
    else
    {
        for (unsigned i = 0; i < n; ++i)
        {
            auto sample_position = ray.m_origin + ray.m_direction * (a + step * (i + 0.5));
            double sample_height = sampleHeight(sample_position);

            data[0].rayleigh += glm::exp(-sample_height / h_rayleigh) * step;
            data[0].mie      += glm::exp(-sample_height / h_mie)      * step;
        }  
    }

    return data;
}
