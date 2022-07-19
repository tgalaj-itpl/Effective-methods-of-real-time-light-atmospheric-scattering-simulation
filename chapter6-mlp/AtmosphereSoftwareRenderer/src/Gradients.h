#pragma once

#include "Vertex.h"

class Gradients
{
public:
    Gradients(const Vertex& min_y, const Vertex& mid_y, const Vertex& max_y);
    ~Gradients();

    float getTexcoordX(int loc) const { return m_texcoord_x[loc]; }
    float getTexcoordY(int loc) const { return m_texcoord_y[loc]; }
    float getOneOverZ (int loc) const { return m_one_over_z[loc]; }
    float getDepth    (int loc) const { return m_depth[loc]; }
    float getLightAmt (int loc) const { return m_light_amt[loc]; }
          
    float getTexcoordStepXX() const { return m_texcoord_xx_step; }
    float getTexcoordStepXY() const { return m_texcoord_xy_step; }
    float getTexcoordStepYX() const { return m_texcoord_yx_step; }
    float getTexcoordStepYY() const { return m_texcoord_yy_step; }
    float getOneOverZStepX()  const { return m_one_over_z_step_x; }
    float getOneOverZStepY()  const { return m_one_over_z_step_y; }
    float getDepthStepX()     const { return m_depth_step_x; }
    float getDepthStepY()     const { return m_depth_step_y; }
    float getLightAmtStepX()  const { return m_light_amt_step_x; }
    float getLightAmtStepY()  const { return m_light_amt_step_y; }

private:
    float* m_texcoord_x;
    float* m_texcoord_y;
    float* m_one_over_z;
    float* m_depth;
    float* m_light_amt;

    float m_texcoord_xx_step;
    float m_texcoord_xy_step;
    float m_texcoord_yx_step;
    float m_texcoord_yy_step;

    float m_one_over_z_step_x;
    float m_one_over_z_step_y;

    float m_depth_step_x;
    float m_depth_step_y;

    float m_light_amt_step_x;
    float m_light_amt_step_y;

    float calcStepX(float* values, const Vertex& min_y, const Vertex& mid_y, const Vertex& max_y, float one_over_dx) const;
    float calcStepY(float* values, const Vertex& min_y, const Vertex& mid_y, const Vertex& max_y, float one_over_dy) const;
};
