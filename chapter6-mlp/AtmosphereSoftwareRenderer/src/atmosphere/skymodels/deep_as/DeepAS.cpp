#include "DeepAS.h"
#include "atmosphere/raytracer/Utils.h"
#include "atmosphere/MemoryMapped.h"
#include "Timing.h"

#include <sstream>
#include <iomanip>
#include <sstream>
#include <glm/gtc/epsilon.hpp>

DeepAS::DeepAS(const Options & options)
    : Atmosphere(options)
{
    std::cout << "DEEP ATMOSPHERIC SCATTERING INFO" << std::endl;
        
    auto tmp_beta_r = BETA_RAYLEIGH / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);
    auto tmp_beta_m = BETA_MIE      / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);

    std::cout << "PLANET R             = " << planet_radius / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "ATMOSPHERE THICKNESS = " << (atmosphere_radius - planet_radius) / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_r                  = " << h_rayleigh / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_m                  = " << h_mie / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "BETA_r               = " << "(" << tmp_beta_r.x << ", " << tmp_beta_r.y << "," << tmp_beta_r.z << ")" << std::endl;
    std::cout << "BETA_m               = " << "(" << tmp_beta_m.x << ", " << tmp_beta_m.y << "," << tmp_beta_m.z << ")" << std::endl << std::endl;

    m_opt = options;
}

glm::highp_dvec3 DeepAS::computeIncidentLight(const Ray & ray, double t_min, double t_max)
{
    double t0, t1;
    if (!intersect(ray, t0, t1) || t1 < 0.0)
    {
        return glm::highp_dvec3(0.0);
    }

    if (t0 > t_min && t0 > 0.0)
    {
        t_min = t0;
    }

    if (t1 < t_max)
    {
        t_max = t1;
    }

    /* mu in the paper which is the cosine of the angle between the sun direction and the ray direction */
    double mu = glm::dot(ray.m_direction, sun_light);
    double phase_r = rayleigh_phase_func(mu);
    double phase_m = mie_phase_func(g, mu);

    glm::highp_dvec3 P_v = ray.m_origin;
    glm::highp_dvec3 P_g = ray.m_origin + ray.m_direction * t_max;
    glm::highp_dvec3 R_v = ray.m_direction;

    if (t0 > 0.0)
    {
        P_v = ray.m_origin + ray.m_direction * t0;
    }

    /* Calculate light intensity for the P_v */
    double view_angle = glm::acos(glm::dot(glm::normalize(P_v), R_v));
    double sun_angle  = glm::acos(glm::dot(glm::normalize(P_v), sun_light));
    double height     = sampleHeight(P_v);
    
#if 1 // single planet
    auto in = keras2cpp::Tensor{ 3 };
    in.data_ = { float(height / (atmosphere_radius - planet_radius)), float(sun_angle / glm::pi<double>()), float(view_angle / glm::pi<double>()) };
#else //multi planets
    constexpr float max_planet_radius = 6360e3f;
    constexpr float planet_radius = 6360e3f;
    constexpr float atmo_radius = planet_radius + 100e3;

    auto in = keras2cpp::Tensor{ 5 };
    in.data_ = { float(height / (atmosphere_radius - planet_radius)), 
                 float(sun_angle / glm::pi<double>()), 
                 float(view_angle / glm::pi<double>()),
                 planet_radius / max_planet_radius,
                 atmo_radius / max_planet_radius};
#endif
    auto out = m_neural_network(in);

    auto intensity_rayleigh = glm::highp_dvec4(out.data_[0], out.data_[1], out.data_[2], out.data_[3]);
    auto intensity_mie      = glm::highp_dvec3(intensity_rayleigh) * intensity_rayleigh.a * BETA_RAYLEIGH.r * BETA_MIE / (intensity_rayleigh.r * BETA_MIE.r * BETA_RAYLEIGH + 0.00001);
    
    auto atmo_color = (glm::highp_dvec3(intensity_rayleigh) * phase_r + intensity_mie * phase_m) * sun_intensity;

    return atmo_color;
}

std::vector<IntegrationData> DeepAS::integrator(Ray ray, double a, double b, unsigned n, bool precomptute)
{
    std::vector<IntegrationData> data(1);
    return data;
}