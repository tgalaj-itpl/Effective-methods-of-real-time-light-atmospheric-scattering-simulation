#pragma once

#include <stdint.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Ray.h"

class Camera
{
public:
    Camera(uint32_t width, uint32_t height, const glm::highp_dvec3 & eye, double fov, double pitch, double yaw);
    
    Ray getPrimaryRay(double x, double y);

    uint32_t m_width, m_height;
    glm::highp_dvec3 m_eye;
    double m_fov;
    double m_aspect_ratio;
    bool m_use_fisheye;

private:
    glm::highp_mat4 m_cam_transform;
    double m_angle;
};
