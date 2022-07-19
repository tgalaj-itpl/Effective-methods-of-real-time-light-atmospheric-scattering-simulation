#include "ImgBased.h"
#include "raytracer/Utils.h"
#include "MemoryMapped.h"
#include "raytracer/Timing.h"

#include <sstream>
#include <iomanip>
#include <sstream>
#include <glm/gtc/epsilon.hpp>

ImgBased::ImgBased(const Options & options)
    : Atmosphere(options)
{
    std::cout << "IMG BASED SCATTERING INFO" << std::endl;
        
    auto tmp_beta_r = BETA_RAYLEIGH / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);
    auto tmp_beta_m = BETA_MIE      / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);

    std::cout << "PLANET R             = " << planet_radius / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "ATMOSPHERE THICKNESS = " << (atmosphere_radius - planet_radius) / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_r                  = " << h_rayleigh / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_m                  = " << h_mie / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "BETA_r               = " << "(" << tmp_beta_r.x << ", " << tmp_beta_r.y << "," << tmp_beta_r.z << ")" << std::endl;
    std::cout << "BETA_m               = " << "(" << tmp_beta_m.x << ", " << tmp_beta_m.y << "," << tmp_beta_m.z << ")" << std::endl;

    std::cout << "Model size in bytes  = " << sizeof(m_neural_network) << std::endl << std::endl;

    m_opt = options;
}

glm::highp_dvec3 ImgBased::computeIncidentLight(const Ray & ray, double t_min, double t_max)
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

    glm::highp_dvec3 P_v = ray.m_origin;
    glm::highp_dvec3 R_v = ray.m_direction;
    R_v.y = -R_v.y;
    R_v = glm::normalize(R_v);

    if (t0 >= 0.0)
    {
        P_v = ray.m_origin + ray.m_direction * t_min;
    }

    /* Calculate light intensity for the P_v */
    double sun_angle = glm::acos(glm::dot(glm::normalize(P_v), sun_light));
    
    auto in = keras2cpp::Tensor{ 5 };
    in.data_ = { float( m_zenith_angle / glm::pi<double>()), float(m_azimuth_angle / glm::pi<double>()), float(R_v.x), float(R_v.y), float(R_v.z) };
    auto out = m_neural_network(in);

    auto predicted_pixel = glm::highp_dvec3(out.data_[0], out.data_[1], out.data_[2]);
    auto atmo_color = 10.0 * predicted_pixel / (1.0 - predicted_pixel);

    return atmo_color;
}

std::vector<IntegrationData> ImgBased::integrator(Ray ray, double a, double b, unsigned n, bool precomptute)
{
    std::vector<IntegrationData> data(1);
    return data;
}