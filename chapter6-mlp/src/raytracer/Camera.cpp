#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

Camera::Camera(uint32_t width, uint32_t height, const glm::highp_dvec3 & eye, double fov, double pitch, double yaw)
    : m_width(width),
      m_height(height),
      m_fov(fov),
      m_aspect_ratio(double(width) / double(height)),
      m_use_fisheye(false)
{
    m_angle = glm::tan(glm::radians(m_fov / 2.0f));
    m_eye = eye;

    yaw   = glm::radians(-90.0 - yaw);
    pitch = glm::radians(-pitch);

    glm::highp_dvec3 dir   = glm::highp_dvec3(glm::cos(yaw) * glm::cos(pitch),
                                              glm::sin(pitch),
                                              glm::sin(yaw) * glm::cos(pitch));

    m_cam_transform = glm::lookAt(eye, eye + dir, glm::highp_dvec3(0, -1, 0));
}

Ray Camera::getPrimaryRay(double x, double y)
{
    Ray primary_ray;

    if (m_use_fisheye)
    {
        double fisheye_cam_x = 2.0 * x / float(m_width  - 1) - 1.0;
        double fisheye_cam_y = 2.0 * y / float(m_height - 1) - 1.0;
        double z2 = fisheye_cam_x * fisheye_cam_x + fisheye_cam_y * fisheye_cam_y;

        if (z2 <= 1) 
        {
            double phi   = std::atan2(fisheye_cam_y, fisheye_cam_x);
            double theta = std::acos(1.0 - z2);
            primary_ray.m_origin    = m_eye;
            primary_ray.m_direction = glm::highp_dvec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
            primary_ray.m_direction = glm::highp_dvec3(m_cam_transform * glm::highp_dvec4(primary_ray.m_direction, 0.0));
            primary_ray.m_direction = glm::normalize(primary_ray.m_direction);
            primary_ray.is_valid    = true;
        }
    }
    else
    {
        double pixel_cam_x = (2.0f * ((x) / m_width) - 1.0f) * m_aspect_ratio * m_angle;
        double pixel_cam_y = (1.0f - 2.0f * ((y) / m_height)) * m_angle;

        primary_ray.m_origin    = m_eye;
        primary_ray.m_direction = glm::highp_dvec3(pixel_cam_x, pixel_cam_y, -1.0);
        primary_ray.m_direction = glm::highp_dvec3(m_cam_transform * glm::highp_dvec4(primary_ray.m_direction, 0.0));
        primary_ray.m_direction = glm::normalize(primary_ray.m_direction);
        primary_ray.is_valid    = true;
    }

    return primary_ray;
}
