#pragma once

#include <stdint.h>
#include <glm/glm.hpp>
#include "Ray.h"
#include <atmosphere\raytracer\Options.h>

class Camera
{
public:
    Camera(uint32_t width, uint32_t height, const glm::highp_dvec3 & eye, double fov, double pitch, double yaw, Options options);
    
    Ray getPrimaryRay(double x, double y);
    void setDirection(double yaw, double pitch);
    void setPosition(const glm::highp_dvec3& new_pos);


    uint32_t m_width, m_height;
    double m_fov;
    double m_aspect_ratio;
    bool m_use_fisheye;

private:
    glm::highp_mat4 m_view;
    glm::highp_mat4 m_inv_view;
    glm::highp_mat4 m_proj;
    glm::highp_mat4 m_inv_proj;
    glm::highp_dvec3 m_eye;
    glm::highp_dvec3 m_dir;
    Options m_options;

    double m_angle;
};
