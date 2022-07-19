#include "Gradients.h"

Gradients::Gradients(const Vertex & min_y, const Vertex & mid_y, const Vertex & max_y)
{
    float one_over_dx = 1.0f / (((mid_y.m_pos.x - max_y.m_pos.x) * 
                                 (min_y.m_pos.y - max_y.m_pos.y)) - 
                                ((min_y.m_pos.x - max_y.m_pos.x) *
                                 (mid_y.m_pos.y - max_y.m_pos.y)));

    float one_over_dy = -one_over_dx;

    m_texcoord_x = new float[3];
    m_texcoord_y = new float[3];
    m_one_over_z = new float[3];
    m_depth      = new float[3];
    m_light_amt  = new float[3];

    m_one_over_z[0] = 1.0f / min_y.m_pos.w;
    m_one_over_z[1] = 1.0f / mid_y.m_pos.w;;
    m_one_over_z[2] = 1.0f / max_y.m_pos.w;;

    m_texcoord_x[0] = min_y.m_texcoords.x * m_one_over_z[0];
    m_texcoord_x[1] = mid_y.m_texcoords.x * m_one_over_z[1];
    m_texcoord_x[2] = max_y.m_texcoords.x * m_one_over_z[2];
    
    m_texcoord_y[0] = min_y.m_texcoords.y * m_one_over_z[0];
    m_texcoord_y[1] = mid_y.m_texcoords.y * m_one_over_z[1];
    m_texcoord_y[2] = max_y.m_texcoords.y * m_one_over_z[2];

    m_depth[0] = min_y.m_pos.z;
    m_depth[1] = mid_y.m_pos.z;
    m_depth[2] = max_y.m_pos.z;

    glm::vec4 light_dir { 0, 0, 1, 0};
    //glm::vec4 dir_to_eye {0, 0, 1, 0};
    //
    //glm::vec4 halfDirection = glm::normalize(dir_to_eye + light_dir);

    //float specular_min = glm::pow(glm::max(glm::dot(halfDirection, min_y.m_normal), 0.0f), 400.0f);
    //float specular_mid = glm::pow(glm::max(glm::dot(halfDirection, mid_y.m_normal), 0.0f), 400.0f);
    //float specular_max = glm::pow(glm::max(glm::dot(halfDirection, max_y.m_normal), 0.0f), 400.0f);

    m_light_amt[0] = /*glm::clamp(*/glm::clamp(glm::dot(min_y.m_normal, light_dir), 0.1f, 0.9f);// + specular_min*20000000.0f, 0.0f, 1.0f);
    m_light_amt[1] = /*glm::clamp(*/glm::clamp(glm::dot(mid_y.m_normal, light_dir), 0.1f, 0.9f);// + specular_mid*20000000.0f, 0.0f, 1.0f);
    m_light_amt[2] = /*glm::clamp(*/glm::clamp(glm::dot(max_y.m_normal, light_dir), 0.1f, 0.9f);// + specular_max*20000000.0f, 0.0f, 1.0f);

    m_texcoord_xx_step  = calcStepX(m_texcoord_x, min_y, mid_y, max_y, one_over_dx);
    m_texcoord_xy_step  = calcStepY(m_texcoord_x, min_y, mid_y, max_y, one_over_dy);
    m_texcoord_yx_step  = calcStepX(m_texcoord_y, min_y, mid_y, max_y, one_over_dx);
    m_texcoord_yy_step  = calcStepY(m_texcoord_y, min_y, mid_y, max_y, one_over_dy);
    m_one_over_z_step_x = calcStepX(m_one_over_z, min_y, mid_y, max_y, one_over_dx);
    m_one_over_z_step_y = calcStepY(m_one_over_z, min_y, mid_y, max_y, one_over_dy);
    m_depth_step_x      = calcStepX(m_depth,      min_y, mid_y, max_y, one_over_dx);
    m_depth_step_y      = calcStepY(m_depth,      min_y, mid_y, max_y, one_over_dy);
    m_light_amt_step_x  = calcStepX(m_light_amt,  min_y, mid_y, max_y, one_over_dx);
    m_light_amt_step_y  = calcStepY(m_light_amt,  min_y, mid_y, max_y, one_over_dy);
}

Gradients::~Gradients()
{
    if(m_texcoord_x != nullptr)
    {
        delete[] m_texcoord_x;
    }

    if (m_texcoord_y != nullptr)
    {
        delete[] m_texcoord_y;
    }

    if (m_one_over_z != nullptr)
    {
        delete[] m_one_over_z;
    }

    if (m_depth != nullptr)
    {
        delete[] m_depth;
    }

    if (m_light_amt != nullptr)
    {
        delete[] m_light_amt;
    }
}

float Gradients::calcStepX(float * values, const Vertex & min_y, const Vertex & mid_y, const Vertex & max_y, float one_over_dx) const
{
    return (((values[1] - values[2]) *
             (min_y.m_pos.y - max_y.m_pos.y)) -
             ((values[0] - values[2]) *
             (mid_y.m_pos.y - max_y.m_pos.y))) * one_over_dx;
}

float Gradients::calcStepY(float* values, const Vertex& min_y, const Vertex& mid_y, const Vertex& max_y, float one_over_dy) const
{
    return (((values[1] - values[2]) *
             (min_y.m_pos.x - max_y.m_pos.x)) -
             ((values[0] - values[2]) *
             (mid_y.m_pos.x - max_y.m_pos.x))) * one_over_dy;
}
