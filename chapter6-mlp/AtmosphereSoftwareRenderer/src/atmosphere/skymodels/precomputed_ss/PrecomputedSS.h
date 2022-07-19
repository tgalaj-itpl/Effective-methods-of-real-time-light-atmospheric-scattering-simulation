#pragma once
#include "atmosphere/skymodels/Atmosphere.h"
#include "atmosphere/raytracer/Options.h"
#include <fstream>

class PrecomputedSS : public Atmosphere
{
public:
    explicit PrecomputedSS(const Options & options, uint32_t _samples = 16, uint32_t _samples_light = 8, const std::string & output_filename = "");
    ~PrecomputedSS() = default;

    glm::highp_dvec3 computeIncidentLight(const Ray & ray, double t_min, double t_max) override;

    void precomputeSingleScattering(int view_angle_samples, int sun_angle_samples, int height_samples);
    void saveSingleScatteringLUT(const std::string & filename);
    bool loadSingleScatteringLUT(const std::string & filename);

    void loadSingleScatteringLUT(const std::vector<std::vector<std::vector<glm::highp_dvec4>>> & data);
    void loadSingleScatteringLUTToVector(const std::string & filename);

    std::vector<std::vector<std::vector<glm::highp_dvec4>>> getLUT();

private:
    std::vector<IntegrationData> integrator(Ray ray, double a, double b, unsigned n, bool precomptute) override;
    bool computeSunLight(Ray light_ray, double & optical_depth_light_r, double & optical_depth_light_m);

    void calculateSingleScattering(double view_angle, double sun_angle, double height, glm::highp_dvec3 & out_rayleigh, glm::highp_dvec3 & out_mie);
    void integrateSingleScattering(const Ray & ray, double t_min, double t_max, glm::highp_dvec3 & out_rayleigh, glm::highp_dvec3 & out_mie);

    glm::highp_dvec4 bilinearInterpolation(double tx, double ty, const glm::highp_dvec4 & c00, const glm::highp_dvec4 & c10, const glm::highp_dvec4 & c01, const glm::highp_dvec4 & c11);
    glm::highp_dvec4 trilinearInterpolation(double tx, double ty, double tz, const glm::highp_dvec4 & c000, const glm::highp_dvec4 & c100, const glm::highp_dvec4 & c010, const glm::highp_dvec4 & c110,
                                                                             const glm::highp_dvec4 & c001, const glm::highp_dvec4 & c101, const glm::highp_dvec4 & c011, const glm::highp_dvec4 & c111);

    std::vector<std::vector<std::vector<glm::highp_dvec4>>> m_single_scattering_data_r_m;

    int m_view_angle_samples;
    int m_sun_angle_samples;
    int m_height_samples;
    Options m_opt;

    std::ofstream output_file;
};

