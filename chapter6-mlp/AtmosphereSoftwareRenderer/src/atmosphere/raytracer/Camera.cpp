#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

Camera::Camera(uint32_t width, uint32_t height, const glm::highp_dvec3 & eye, double fov, double pitch, double yaw, Options options)
    : m_width(width),
      m_height(height),
      m_fov(fov),
      m_aspect_ratio(double(width) / double(height)),
      m_use_fisheye(false),
      m_options(options)
{
    m_angle = glm::tan(glm::radians(m_fov / 2.0f));
    
    setPosition(eye);
    setDirection(yaw, pitch);
}

Ray Camera::getPrimaryRay(double x, double y)
{
    Ray primary_ray;

    if (m_use_fisheye)
    {
        double fisheye_cam_x = 2.0 * x / float(m_width  - 1.0) - 1.0;
        double fisheye_cam_y = 2.0 * y / float(m_height - 1.0) - 1.0;
        double z2 = fisheye_cam_x * fisheye_cam_x + fisheye_cam_y * fisheye_cam_y;

        if (z2 < 1.0) 
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

void Camera::setDirection(double yaw, double pitch)
{
    auto _yaw   = glm::radians(yaw);
    auto _pitch = glm::radians(pitch);

    m_dir   = glm::highp_dvec3(glm::cos(_yaw) * glm::cos(_pitch),
                               glm::sin(_pitch),
                               glm::sin(_yaw) * glm::cos(_pitch));
    m_dir = glm::normalize(m_dir);

    m_cam_transform = glm::lookAt(m_eye, m_eye + m_dir, glm::highp_dvec3(0, -1, 0));
}

void Camera::setPosition(const glm::highp_dvec3& new_pos)
{
    m_eye = (new_pos - glm::highp_dvec3(0.0f, m_options.PLANET_RADIUS + 1000.0f, 0.0f)) * m_options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR;
    m_cam_transform = glm::lookAt(m_eye, m_eye + m_dir, glm::highp_dvec3(0, -1, 0));
}
