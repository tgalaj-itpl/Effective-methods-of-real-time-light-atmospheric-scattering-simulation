#include "Edge.h"

Edge::Edge(const Gradients& gradients, const Vertex& start, const Vertex& end, int min_y_index)
{
    m_y_start = static_cast<int>(glm::ceil(start.m_pos.y));
    m_y_end   = static_cast<int>(glm::ceil(end.m_pos.y));

    float y_dist = end.m_pos.y - start.m_pos.y;
    float x_dist = end.m_pos.x - start.m_pos.x;

    float y_prestep = m_y_start - start.m_pos.y;

    m_x_step = x_dist / y_dist;
    m_x = start.m_pos.x + y_prestep * m_x_step;

    float x_prestep = m_x - start.m_pos.x;
    m_texcoord_x      = gradients.getTexcoordX(min_y_index) + (gradients.getTexcoordStepXX() * x_prestep) + (gradients.getTexcoordStepXY() * y_prestep);
    m_texcoord_x_step = gradients.getTexcoordStepXY() + (gradients.getTexcoordStepXX() * m_x_step);

    m_texcoord_y      = gradients.getTexcoordY(min_y_index) + (gradients.getTexcoordStepYX() * x_prestep) + (gradients.getTexcoordStepYY() * y_prestep);
    m_texcoord_y_step = gradients.getTexcoordStepYY() + (gradients.getTexcoordStepYX() * m_x_step);

    m_one_over_z      = gradients.getOneOverZ(min_y_index) + 
                       (gradients.getOneOverZStepX() * x_prestep) + 
                       (gradients.getOneOverZStepY() * y_prestep);

    m_one_over_z_step = gradients.getOneOverZStepY() + (gradients.getOneOverZStepX() * m_x_step);

    m_depth = gradients.getDepth(min_y_index) + 
             (gradients.getDepthStepX() * x_prestep) + 
             (gradients.getDepthStepY() * y_prestep);;
    m_depth_step = gradients.getDepthStepY() + (gradients.getDepthStepX() * m_x_step);

    m_light_amt = gradients.getLightAmt(min_y_index) +
                 (gradients.getLightAmtStepX() * x_prestep) +
                 (gradients.getLightAmtStepY() * y_prestep);;
    m_light_amt_step = gradients.getLightAmtStepY() + (gradients.getLightAmtStepX() * m_x_step);
}

void Edge::step()
{
    m_x          += m_x_step;
    m_texcoord_x += m_texcoord_x_step;
    m_texcoord_y += m_texcoord_y_step;
    m_one_over_z += m_one_over_z_step;
    m_depth      += m_depth_step;
    m_light_amt  += m_light_amt_step;
}
