#include "Atmosphere.h"
#include "raytracer/Utils.h"
#include "raytracer/Framebuffer.h"
#include <limits>

glm::highp_dvec3 Atmosphere::computeIncidentLight(const Ray& ray, double t_min, double t_max)
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

    // t_min, t_max - points that intersect with atmosphere;

    double segment_length = (t_max - t_min) / samples;
    double t_current = t_min;

    /* Rayleigh and Mie contributions */
    glm::highp_dvec3 sum_r(0.0f), sum_m(0.0f);
    double optical_depth_r = 0, optical_depth_m = 0;

    /* mu in the paper which is the cosine of the angle between the sun direction and the ray direction */
    double mu = glm::dot(ray.m_direction, sun_light);

    double phase_r = rayleigh_phase_func(mu);
    double phase_m = mie_phase_func(g, mu);

    /* Precompute the outer integral */
    auto h_r_m = integrator(ray, t_min, t_max, samples, true);

    /* Compute inner integrals */
    for (unsigned i = 0; i < samples + samples_modifier; ++i)
    {
        optical_depth_r += h_r_m[i].rayleigh;
        optical_depth_m += h_r_m[i].mie;

        Ray light_ray(h_r_m[i].sample_position, sun_light);

        double t0_light, t1_light;
        intersect(light_ray, t0_light, t1_light);

        auto optical_depth_light = integrator(light_ray, 0.0, t1_light, samples_light, false);

        glm::highp_dvec3 tau = BETA_RAYLEIGH  * (1.0 * optical_depth_r + 1.0 * optical_depth_light[0].rayleigh) + 
                               BETA_MIE       * (1.0 * optical_depth_m + 1.0 * optical_depth_light[0].mie);
        glm::highp_dvec3 attenuation = glm::exp(-tau);

        sum_r += attenuation * h_r_m[i].rayleigh;
        sum_m += attenuation * h_r_m[i].mie;
    }

    return (sum_r * BETA_RAYLEIGH * phase_r + sum_m * BETA_MIE * phase_m) * sun_intensity;
}

bool Atmosphere::intersect(const Ray & ray, double & t0, double & t1, bool is_planet)
{
    const double radius = is_planet ? planet_radius : atmosphere_radius;

    // They ray dir is normalized so A = 1 
    double A = glm::dot(ray.m_direction, ray.m_direction);
    double B = 2.0 * glm::dot(ray.m_direction, ray.m_origin);
    double C = glm::dot(ray.m_origin, ray.m_origin) - radius * radius;
 
    if (!solveQuadraticEquation(A, B, C, t0, t1)) return false; 
 
    if (t0 > t1) std::swap(t0, t1); 
 
    return true; 
}

double Atmosphere::sampleHeight(glm::highp_dvec3 & pos)
{
    return glm::length(pos) - planet_radius;
}
