#pragma once

#include <thread>
#include <atomic>

#include "skymodels/Atmosphere.h"
#include "Scene.h"
#include "Camera.h"
#include "Options.h"

#define USE_TONE_MAPPING true

class Framebuffer
{
public:
    enum class ColorScaleBarType { LUMINANCE, RMSE };
    Framebuffer(const Options & options);
    ~Framebuffer();

    void render(Atmosphere & atmosphere);
    std::vector<glm::highp_dvec3> getFramebufferRawData() const;    
    void saveRender(bool tonemap = true, double exposure = 1.0) const;
    
    void saveImg(const std::string & filename, const std::vector<glm::highp_dvec3>& data);
    std::vector<glm::highp_dvec3> loadImg(const std::string& filename);
    std::vector<glm::highp_dvec3> getLuminance(const std::vector<glm::highp_dvec3>& data);
    std::vector<glm::highp_dvec3> getChromaticity(const std::vector<glm::highp_dvec3>& data);
    std::vector<glm::highp_dvec3> getMSE(const std::vector<glm::highp_dvec3>& ref, const std::vector<glm::highp_dvec3>& src, double & mse_error, double scale=40.0);

    static std::vector<glm::highp_dvec3> getLuminance(uint32_t width, uint32_t height, const std::vector<glm::highp_dvec3>& data);
    static std::vector<glm::highp_dvec3> getChromaticity(uint32_t width, uint32_t height, const std::vector<glm::highp_dvec3>& data);
    static void saveImg(const std::string& filename, const uint32_t width, const uint32_t height, const std::vector<glm::highp_dvec3>& data);
    static std::vector<glm::highp_dvec3> loadImg(const std::string & filename, int& width, int& height);

    static void createColorScaleBar(const std::string& output_filename, ColorScaleBarType type, unsigned width);

    Options m_options;

    static bool FLIP_HORIZONTALLY;
    static bool FLIP_VERTICALLY;

private:
    void process(Atmosphere & atmosphere, int left, int right);
    std::vector<int> thread_bounds(int parts, int mem) const;

    glm::highp_dvec3 * m_framebuffer;
    Camera * m_cam;

    std::vector<std::thread> m_threads;
    std::atomic<int> m_processed_pixel_counter;

    int m_width, m_height;
};
