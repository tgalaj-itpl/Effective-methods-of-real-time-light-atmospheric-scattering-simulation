#pragma once
#include "Vertex.h"
#include "Gradients.h"

class Edge
{
public:
    Edge(const Gradients& gradients, const Vertex& start, const Vertex& end, int min_y_index);

    void step();

    int getStart() const { return m_y_start; }
    int getEnd()   const { return m_y_end; }
    
    float getX()         const { return m_x; }
    float getTexcoordX() const { return m_texcoord_x; }
    float getTexcoordY() const { return m_texcoord_y; }
    float getOneOverZ()  const { return m_one_over_z; }
    float getDepth()     const { return m_depth; }
    float getLightAmt()  const { return m_light_amt; }

private:
    float m_texcoord_x;
    float m_texcoord_x_step;
    float m_texcoord_y;
    float m_texcoord_y_step;
    float m_one_over_z;
    float m_one_over_z_step;
    float m_depth;
    float m_depth_step;
    float m_light_amt;
    float m_light_amt_step;

    float m_x;
    float m_x_step;
    int m_y_start;
    int m_y_end;
};
