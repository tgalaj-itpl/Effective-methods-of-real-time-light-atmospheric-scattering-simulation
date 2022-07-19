#pragma once

#include "RenderContext.h"

#include <glm/vec3.hpp>
#include <atmosphere/raytracer/Camera.h>
#include <atmosphere/skymodels/precomputed_ss/PrecomputedSS.h>
#include <atmosphere/skymodels/Atmosphere.h>

class AtmoElek
{
public:
    AtmoElek() = delete;
    AtmoElek(const Options & options);
    ~AtmoElek();

    void init(RenderContext* target);
    void updateAndRender(RenderContext* target, float delta);
    
    void setSunDirection(double angle);
    void setCameraDir(double yaw, double pitch);
    void setCameraPosition(glm::highp_dvec3 new_position);

private:
    void process(const RenderContext * const target, int left, int right, float delta);
    std::vector<int> thread_bounds(int parts, int mem) const;
    glm::highp_ivec3 tonemap(const glm::highp_dvec3& hdr_color, double exposure = 1.0, double gamma = 2.2);

    std::vector<std::thread> m_threads;
    std::vector<int> m_bounds;
    int m_num_threads;

    Options m_options;
    PrecomputedSS * m_atmosphere;
    Camera * m_cam;
};

