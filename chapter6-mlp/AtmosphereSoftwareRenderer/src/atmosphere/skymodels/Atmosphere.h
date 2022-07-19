#pragma once

#include <glm/glm.hpp>
#include <mutex>

#include "atmosphere/raytracer/Options.h"
#include "atmosphere/raytracer/Ray.h"
#include <vector>
#include <glm/gtc/constants.hpp>

#define USE_M_PHI 0
#define M_PHI 1.618033988749895

#define HG_PHASE_FUNC 0 //Henyey-Greenstein
#define CS_PHASE_FUNC 1 //Cornette-Shanks
#define S_PHASE_FUNC  0 //Schlick

struct IntegrationInfo
{
    double m_rayleigh_optical_depth;
    double m_rayleigh_optical_depth_light;
    double m_mie_optical_depth;
    double m_mie_optical_depth_light;
};


struct IntegrationData
{
    glm::highp_dvec3 sample_position{ 0.0 };

    double mie = 0.0;
    double rayleigh = 0.0;
};

class Atmosphere
{
public:
    explicit Atmosphere(const Options & _options)
        : sun_light(_options.SUN_DIRECTION),
          sun_intensity(_options.SUN_INTENSITY),
          planet_radius(_options.PLANET_RADIUS),
          atmosphere_radius(_options.ATMOSPHERE_RADIUS),
          h_rayleigh(_options.H_RAYLEIGH),
          h_mie(_options.H_MIE),
          g(0.76),
          samples(0), 
          samples_light(0), 
          samples_modifier(0)
    {
        double scaling_factor = _options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR;

        BETA_RAYLEIGH = _options.BETA_RAYLEIGH;
        BETA_MIE      = _options.BETA_MIE;

        planet_radius     *= scaling_factor;
        atmosphere_radius *= scaling_factor;
        h_rayleigh        *= scaling_factor;
        h_mie             *= scaling_factor;
        BETA_RAYLEIGH     *= 1.0 / scaling_factor;
        BETA_MIE          *= 1.0 / scaling_factor;
    }

    virtual ~Atmosphere() = default;

    virtual glm::highp_dvec3 computeIncidentLight(const Ray & ray, double t_min, double t_max);
    virtual bool intersect(const Ray & ray, double & t0, double & t1, bool is_planet = false);

    void setSunDirection(const glm::highp_dvec3 & sun_dir)
    {
        sun_light = glm::normalize(sun_dir);
    }

protected:
    virtual std::vector<IntegrationData> integrator(Ray ray, double a, double b, unsigned n, bool precomptute) = 0;
    virtual double sampleHeight(glm::highp_dvec3 & pos);

    double rayleigh_phase_func(double mu)
    {
        return 3.0 * (1.0 + mu * mu) / (16.0 * glm::pi<double>());
    }

    double rayleigh_phase_func_elek(double mu)
    {
        return 0.7 * (1.4 + 0.5 * mu) / (4.0 * glm::pi<double>());
    }

#if HG_PHASE_FUNC
    /* Henyey-Greenstein Mie Phase Function Approximation */
    double mie_phase_func(double g, double mu)
    {
        return (1.0 - g * g) / (4.0 * glm::pi<double>() * glm::pow(1.0 + g*g - 2*g * mu, 1.5));
    }
#elif CS_PHASE_FUNC
    /* Cornette-Shanks Mie Phase Function Approximation */
    double mie_phase_func(double g, double mu)
    {
        return (3.0 * (1.0 - g*g) * (1.0 + mu*mu)) / (4.0 * glm::pi<double>() * 2.0 * (2.0 + g*g) * pow(1.0 + g*g - 2*g * mu, 1.5));
    }
#elif S_PHASE_FUNC
    /* Schlick Mie Phase Function Approximation */
    double mie_phase_func(double g, double mu)
    {
        const double k = 1.55 * g - 0.55 * g*g*g;

        return (1.0 - k*k) / (4.0 * glm::pi<double>() * (1.0 - k * mu) * (1.0 - k * mu));
    }
#endif

    glm::highp_dvec3 BETA_RAYLEIGH;
    glm::highp_dvec3 BETA_MIE;

    glm::highp_dvec3 sun_light;
    double sun_intensity;

    double planet_radius;
    double atmosphere_radius;
    double h_rayleigh;
    double h_mie;
    double g;
	uint32_t samples, samples_light, samples_modifier;
};
