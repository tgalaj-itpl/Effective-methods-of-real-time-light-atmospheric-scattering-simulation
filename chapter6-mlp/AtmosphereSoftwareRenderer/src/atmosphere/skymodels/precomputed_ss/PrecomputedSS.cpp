#include "PrecomputedSS.h"
#include "atmosphere/raytracer/Utils.h"
#include "atmosphere/MemoryMapped.h"
#include "Timing.h"

#include <sstream>
#include <iomanip>
#include <sstream>
#include <glm/gtc/epsilon.hpp>

PrecomputedSS::PrecomputedSS(const Options & options, uint32_t _samples, uint32_t _samples_light, const std::string & output_filename)
    : Atmosphere(options)
{
    samples       = _samples;
    samples_light = _samples_light;

    std::cout << "PRECOMPUTED SINGLE SCATTERING INFO" << std::endl;
    std::cout << "VIEW SAMPLES  = " << samples << std::endl;
    std::cout << "LIGHT SAMPLES = " << samples_light << std::endl << std::endl;
        
    auto tmp_beta_r = BETA_RAYLEIGH / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);
    auto tmp_beta_m = BETA_MIE      / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);

    std::cout << "PLANET R             = " << planet_radius / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "ATMOSPHERE THICKNESS = " << (atmosphere_radius - planet_radius) / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_r                  = " << h_rayleigh / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_m                  = " << h_mie / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "BETA_r               = " << "(" << tmp_beta_r.x << ", " << tmp_beta_r.y << "," << tmp_beta_r.z << ")" << std::endl;
    std::cout << "BETA_m               = " << "(" << tmp_beta_m.x << ", " << tmp_beta_m.y << "," << tmp_beta_m.z << ")" << std::endl << std::endl;

    m_opt = options;

    /* If output_filename is not empty, it indicates that user wants to create dataset based on generated images */
    if (!output_filename.empty())
    {
        output_file.open(output_filename.c_str());

        if (!output_file.good())
        {
            std::cerr << "Could not open file " << output_filename << std::endl;
        }
    }
}

glm::highp_dvec3 PrecomputedSS::computeIncidentLight(const Ray & ray, double t_min, double t_max)
{
    if (m_single_scattering_data_r_m.empty())
    {
        std::cerr << "Single scattering LUT is empty!" << std::endl;
        return glm::highp_dvec3(0.0);
    }

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

    int view_angle_idx = view_angle * (m_view_angle_samples - 1.0) / glm::pi<double>();
    int sun_angle_idx  = sun_angle  * (m_sun_angle_samples  - 1.0) / glm::pi<double>();
    int h_idx          = height     * (m_height_samples     - 1.0) / (atmosphere_radius - planet_radius);

    /* Interpolate values for angles and height */
    auto view_angle_idx_tmp = (view_angle_idx + 1) >= m_view_angle_samples ? view_angle_idx - 1 : view_angle_idx + 1;
    auto sun_angle_idx_tmp  = (sun_angle_idx  + 1) >= m_sun_angle_samples  ? sun_angle_idx  - 1 : sun_angle_idx  + 1;
    auto h_idx_tmp          = (h_idx          + 1) >= m_height_samples     ? h_idx          - 1 : h_idx          + 1;

    auto view_angle_1_tmp = glm::pi<double>() * view_angle_idx     / (m_view_angle_samples - 1.0);
    auto view_angle_2_tmp = glm::pi<double>() * view_angle_idx_tmp / (m_view_angle_samples - 1.0);

    auto sun_angle_1_tmp = glm::pi<double>() * sun_angle_idx     / (m_sun_angle_samples - 1.0);
    auto sun_angle_2_tmp = glm::pi<double>() * sun_angle_idx_tmp / (m_sun_angle_samples - 1.0);

    auto height_1_tmp = ((atmosphere_radius - planet_radius) * h_idx)     / (m_height_samples - 1.0);
    auto height_2_tmp = ((atmosphere_radius - planet_radius) * h_idx_tmp) / (m_height_samples - 1.0);

    auto tx = (height_1_tmp - height) / (height_1_tmp - height_2_tmp);
    auto ty = (sun_angle_1_tmp   - sun_angle)  / (sun_angle_1_tmp  - sun_angle_2_tmp);
    auto tz = (view_angle_1_tmp - view_angle)  / (view_angle_1_tmp - view_angle_2_tmp);

    /* Calculate interpolated color value */
    auto intensity_rayleigh = trilinearInterpolation(tx,
                                                     ty, 
                                                     tz,
                                                     m_single_scattering_data_r_m[h_idx    ][sun_angle_idx    ][view_angle_idx],
                                                     m_single_scattering_data_r_m[h_idx_tmp][sun_angle_idx    ][view_angle_idx],
                                                     m_single_scattering_data_r_m[h_idx    ][sun_angle_idx_tmp][view_angle_idx],
                                                     m_single_scattering_data_r_m[h_idx_tmp][sun_angle_idx_tmp][view_angle_idx],
                                                     m_single_scattering_data_r_m[h_idx    ][sun_angle_idx    ][view_angle_idx_tmp],
                                                     m_single_scattering_data_r_m[h_idx_tmp][sun_angle_idx    ][view_angle_idx_tmp],
                                                     m_single_scattering_data_r_m[h_idx    ][sun_angle_idx_tmp][view_angle_idx_tmp],
                                                     m_single_scattering_data_r_m[h_idx_tmp][sun_angle_idx_tmp][view_angle_idx_tmp]);

    auto intensity_mie = glm::highp_dvec3(intensity_rayleigh) * intensity_rayleigh.a * BETA_RAYLEIGH.r * BETA_MIE / (intensity_rayleigh.r * BETA_MIE.r * BETA_RAYLEIGH + 0.00001);
    
    auto atmo_color = (glm::highp_dvec3(intensity_rayleigh) * phase_r + intensity_mie * phase_m) * sun_intensity;

    /* If user expressed their will to generate dataset based on the generated images */
    if (output_file.good())
    {
        auto tmp_color = atmo_color / (atmo_color + 10.0);

        output_file <<  sun_angle  / glm::pi<float>() << " "
                    <<  R_v.x                         << " "
                    << -R_v.y                         << " "
                    <<  R_v.z                         << " "
                    <<  tmp_color.r                   << " "
                    <<  tmp_color.g                   << " "
                    <<  tmp_color.b                   << std::endl;
    }

    return atmo_color;
}

glm::highp_dvec4 PrecomputedSS::bilinearInterpolation(double tx,
                                                      double ty, 
                                                      const glm::highp_dvec4 & c00, 
                                                      const glm::highp_dvec4 & c10, 
                                                      const glm::highp_dvec4 & c01, 
                                                      const glm::highp_dvec4 & c11)
{
    auto a = glm::mix(c00, c10, tx);
    auto b = glm::mix(c01, c11, tx);

    return glm::mix(a, b, ty);
}

glm::highp_dvec4 PrecomputedSS::trilinearInterpolation(double tx,
                                                       double ty,
                                                       double tz,
                                                       const glm::highp_dvec4 & c000,
                                                       const glm::highp_dvec4 & c100,
                                                       const glm::highp_dvec4 & c010,
                                                       const glm::highp_dvec4 & c110,
                                                       const glm::highp_dvec4 & c001,
                                                       const glm::highp_dvec4 & c101, 
                                                       const glm::highp_dvec4 & c011, 
                                                       const glm::highp_dvec4 & c111)
{
    auto e = bilinearInterpolation(tx, ty, c000, c100, c010, c110);
    auto f = bilinearInterpolation(tx, ty, c001, c101, c011, c111);

    return glm::mix(e, f, tz);;
}

std::vector<IntegrationData> PrecomputedSS::integrator(Ray ray, double a, double b, unsigned n, bool precomptute)
{
    std::vector<IntegrationData> data(1);
    return data;
}

void PrecomputedSS::calculateSingleScattering(double view_angle, double sun_angle, double height, glm::highp_dvec3 & out_rayleigh, glm::highp_dvec3 & out_mie)
{
    Ray ray;
    ray.m_origin = glm::highp_dvec3(0.0, height, 0.0);
    ray.m_direction = glm::normalize(glm::highp_dvec3(glm::sin(view_angle), glm::cos(view_angle), 0.0));

    auto prev_sun_dir = sun_light;
    sun_light = glm::normalize(glm::highp_dvec3(glm::sin(sun_angle), glm::cos(sun_angle), 0.0));

    double t0, t1, t_max = std::numeric_limits<double>::max();
    if (intersect(ray, t0, t1, true) && t1 > 0.0)
    {
        t_max = glm::max(0.0, t0);
    }

    integrateSingleScattering(ray, 0.0, t_max, out_rayleigh, out_mie);
    sun_light = prev_sun_dir;
}

void PrecomputedSS::integrateSingleScattering(const Ray & ray, double t_min, double t_max, glm::highp_dvec3 & out_rayleigh, glm::highp_dvec3 & out_mie)
{
    double t0, t1;
    if (!intersect(ray, t0, t1) || t1 < 0.0)
    {
        return;
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

    double step = (t_max - t_min) / samples;

    /* Rayleigh and Mie contributions */
    glm::highp_dvec3 sum_r(0.0), sum_m(0.0);
    double optical_depth_r = 0, optical_depth_m = 0;

    for (unsigned i = 0; i < samples; ++i)
    {
        auto sample_position = ray.m_origin + ray.m_direction * (t_min + step * (i + 0.5));
        double sample_height = sampleHeight(sample_position);

        auto hr = glm::exp(-sample_height / h_rayleigh) * step;
        auto hm = glm::exp(-sample_height / h_mie)      * step;

        optical_depth_r += hr;
        optical_depth_m += hm;

        /* Transmittance light path */
        Ray light_ray(sample_position, sun_light);
        double optical_depth_light_r = 0.0, optical_depth_light_m = 0.0;

        if (computeSunLight(light_ray, optical_depth_light_r, optical_depth_light_m))
        {
            /* Compute total transmittance */
            glm::highp_dvec3 tau = BETA_RAYLEIGH * (optical_depth_r + optical_depth_light_r) +
                                   BETA_MIE * (optical_depth_m + optical_depth_light_m);
            glm::highp_dvec3 attenuation = glm::exp(-tau);

            sum_r += attenuation * hr;
            sum_m += attenuation * hm;
        }
    }

    out_rayleigh = sum_r * BETA_RAYLEIGH;
    out_mie      = sum_m * BETA_MIE;
}

bool PrecomputedSS::computeSunLight(Ray light_ray, double & optical_depth_light_r, double & optical_depth_light_m)
{
    double t0_light, t1_light;
    intersect(light_ray, t0_light, t1_light);

    double step_light = t1_light / samples_light;

    for (unsigned j = 0; j < samples_light; ++j)
    {
        auto sample_position_light = light_ray.m_origin + light_ray.m_direction * (step_light * (j + 0.5));
        double sample_height_light = sampleHeight(sample_position_light);

        if (sample_height_light < 0.0)
            return false;

        optical_depth_light_r += glm::exp(-sample_height_light / h_rayleigh) * step_light;
        optical_depth_light_m += glm::exp(-sample_height_light / h_mie)      * step_light;
    }

    return true;
}

void PrecomputedSS::precomputeSingleScattering(int view_angle_samples, int sun_angle_samples, int height_samples)
{
    constexpr double PI    = glm::pi<double>();
    const     double H_TOP = atmosphere_radius - planet_radius;

    double height     = 0.0;
    double sun_angle  = 0.0;
    double view_angle = 0.0;

    m_view_angle_samples = view_angle_samples;
    m_sun_angle_samples  = sun_angle_samples;
    m_height_samples     = height_samples;

    std::printf("Start Single Scattering precomputations...\n"
                "view angle samples = %d\n"
                "sun angle  samples = %d\n"
                "height     samples = %d\n\n", view_angle_samples, sun_angle_samples, height_samples);
    printProgressBar(0, height_samples, "Precomputing:", "Complete");

    m_single_scattering_data_r_m = std::vector<std::vector<std::vector<glm::highp_dvec4>>>(height_samples, std::vector<std::vector<glm::highp_dvec4>>(sun_angle_samples, std::vector<glm::highp_dvec4>(view_angle_samples, glm::highp_dvec4(0.0))));
    
    auto start_time = Time::getTime();
    std::ostringstream ss;

    glm::highp_dvec3 rayleigh_contirb(0.0);
    glm::highp_dvec3 mie_contirb(0.0);

    /* For every height of the observer */
    for (unsigned h = 0; h < height_samples; ++h)
    {
        /* Calculate single-scattering and store it in LUT */
        height = planet_radius + (H_TOP * h) / (height_samples - 1.0);

        /* For every sun angle (between observer's postion and light direction) */
        for (unsigned s = 0; s < sun_angle_samples; ++s)
        {
            sun_angle = PI * s / (sun_angle_samples - 1.0);

            /* For every view angle (between observer's postion and view direction) */
            for (unsigned v = 0; v < view_angle_samples; ++v)
            {
                view_angle = PI * v / (view_angle_samples - 1.0);

                calculateSingleScattering(view_angle, sun_angle, height, rayleigh_contirb, mie_contirb);

                m_single_scattering_data_r_m[h][s][v] = glm::highp_dvec4(rayleigh_contirb, mie_contirb.r);
            }
        }

        ss.str(std::string());
        ss << std::fixed << std::setprecision(2) << (Time::getTime() - start_time) << "s";
        printProgressBar(h + 1, height_samples, "Precomputing:", "Complete | Total time: " + ss.str());
    }
    std::cout << std::endl;
}

void PrecomputedSS::saveSingleScatteringLUT(const std::string & filename)
{
    std::ofstream file("res/" + filename);

    constexpr double PI    = glm::pi<double>();
    const     double H_TOP = atmosphere_radius - planet_radius;

    double height     = 0.0;
    double sun_angle  = 0.0;
    double view_angle = 0.0;

    if (file.is_open())
    {
        printProgressBar(0, m_single_scattering_data_r_m.size(), "Saving LUTs to a file:", "Complete");

        file << m_single_scattering_data_r_m.size()       << " "
             << m_single_scattering_data_r_m[0].size()    << " "
             << m_single_scattering_data_r_m[0][0].size() << " " << std::endl;
        
        /* For every height of the observer */
        for (unsigned h = 0; h < m_height_samples; ++h)
        {
            printProgressBar(h + 1, m_height_samples, "Saving LUTs to a file:", "Complete");

            /* For every sun angle (between observer's postion and light direction) */
            for (unsigned s = 0; s < m_sun_angle_samples; ++s)
            {
                /* For every view angle (between observer's postion and view direction) */
                for (unsigned v = 0; v < m_view_angle_samples; ++v)
                {
                    /* Save input values - height, sun angle, view angle, planet radius, atmo radius */
                    file << h / double(m_height_samples - 1.0)      << " "
                         << s / double(m_sun_angle_samples - 1.0)   << " "
                         << v / double(m_view_angle_samples - 1.0)  << " "
                         << planet_radius / m_opt.MAX_PLANET_R << " "
                         << atmosphere_radius / m_opt.MAX_PLANET_R << " ";

                    /* Save scattering RGBA values -> RGB - rayleigh, A - Mie */
                    file << m_single_scattering_data_r_m[h][s][v].r << " "
                         << m_single_scattering_data_r_m[h][s][v].g << " "
                         << m_single_scattering_data_r_m[h][s][v].b << " "
                         << m_single_scattering_data_r_m[h][s][v].a << std::endl;
                }
            }
        }
        std::cout << std::endl;
    }
    else
    {
        std::cerr << "Unable to save to a file " << filename << std::endl;
        file.close();
    }
}

void PrecomputedSS::loadSingleScatteringLUTToVector(const std::string & filename)
{
    if (filename.empty())
    {
        fprintf(stderr, "Could not open file %s\n\n", filename.c_str());
        return;
    }

    std::string filetext;
    std::string line;

    MemoryMapped data("res/" + filename, MemoryMapped::WholeFile, MemoryMapped::Normal);

    if (!data.isValid())
    {
        fprintf(stderr, "Could not open file not valid %s\n\n", filename.c_str());
        return;
    }

    m_view_angle_samples = 0;
    m_sun_angle_samples  = 0;
    m_height_samples     = 0;

    std::stringstream file_data;
    file_data << (char*)data.getData();
 
    file_data >> line;
    m_height_samples = std::atoi(line.c_str());

    file_data >> line;
    m_sun_angle_samples = std::atoi(line.c_str());

    file_data >> line;
    m_view_angle_samples = std::atoi(line.c_str());

    m_single_scattering_data_r_m.clear();

    m_single_scattering_data_r_m = std::vector<std::vector<std::vector<glm::highp_dvec4>>>(m_height_samples, std::vector<std::vector<glm::highp_dvec4>>(m_sun_angle_samples, std::vector<glm::highp_dvec4>(m_view_angle_samples, glm::highp_dvec4(0.0))));

    std::cout << "LUT VIEW ANGLE SAMPLES = " << m_view_angle_samples << std::endl;
    std::cout << "LUT SUN ANGLE SAMPLES  = " << m_sun_angle_samples << std::endl;
    std::cout << "LUT ALTITUDE SAMPLES   = " << m_height_samples << std::endl << std::endl;

    auto start_time = Time::getTime();
    std::ostringstream ss;

    printProgressBar(0, m_single_scattering_data_r_m.size(), "Loading from a file:", "Complete | Total time: ");

    std::string r, g, b, a, dummy;
    glm::highp_dvec4 rayleigh_mie_contirb(0.0);

    /* For every height of the observer */
    for (unsigned h = 0; h < m_height_samples; ++h)
    {
        ss.str(std::string());
        ss << std::fixed << std::setprecision(2) << (Time::getTime() - start_time) << "s";

        printProgressBar(h + 1, m_height_samples, "Loading from a file:", "Complete | Total time: " + ss.str());

        /* For every sun angle (between observer's postion and light direction) */
        for (unsigned s = 0; s < m_sun_angle_samples; ++s)
        {
            /* For every view angle (between observer's postion and view direction) */
            for (unsigned v = 0; v < m_view_angle_samples; ++v)
            {
                file_data >> dummy; // h index
                file_data >> dummy; // s index
                file_data >> dummy; // v index
                file_data >> dummy; // planet radius
                file_data >> dummy; // atmo radius

                file_data >> r;
                file_data >> g;
                file_data >> b;
                file_data >> a;

                rayleigh_mie_contirb                  = glm::highp_dvec4(atof_fast(r.c_str()), atof_fast(g.c_str()), atof_fast(b.c_str()), atof_fast(a.c_str()));
                m_single_scattering_data_r_m[h][s][v] = rayleigh_mie_contirb;
            }
        }
    }

    std::cout << std::endl;
}

std::vector<std::vector<std::vector<glm::highp_dvec4>>> PrecomputedSS::getLUT()
{
    return m_single_scattering_data_r_m;
}

bool PrecomputedSS::loadSingleScatteringLUT(const std::string & filename)
{
    loadSingleScatteringLUTToVector(filename);

    if (m_single_scattering_data_r_m.empty())
    {
        return false;
    }

    return true;
}

void PrecomputedSS::loadSingleScatteringLUT(const std::vector<std::vector<std::vector<glm::highp_dvec4>>>& data)
{
    m_height_samples     = data.size();
    m_sun_angle_samples  = data[0].size();
    m_view_angle_samples = data[0][0].size();

    m_single_scattering_data_r_m = data;
}
