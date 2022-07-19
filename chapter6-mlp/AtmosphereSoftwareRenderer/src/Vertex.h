#pragma once

#include <glm/glm.hpp>

class Vertex
{
public:
    Vertex(const glm::vec4 & pos, const glm::vec4 & texcoords, const glm::vec4 & normal);

    float triangleArea(Vertex b, Vertex c) const;
    Vertex transform(const glm::mat4 & transform, const glm::mat4 & normal_transform) const;
    Vertex perspectiveDivide() const;
    Vertex lerp(const Vertex& v, float amt) const;

    float get(int index) const
    {
        switch(index)
        {
            case 0:
                return m_pos.x;
            case 1:
                return m_pos.y;
            case 2:
                return m_pos.z;
            case 3:
                return m_pos.w;
            default: 
                return 1.0f;
        }
    }

    bool isInsideViewFrustum() const
    {
        return glm::abs(m_pos.x) <= glm::abs(m_pos.w) &&
               glm::abs(m_pos.y) <= glm::abs(m_pos.w) &&
               glm::abs(m_pos.z) <= glm::abs(m_pos.w);
    }

    glm::vec4 m_pos;
    glm::vec4 m_texcoords;
    glm::vec4 m_normal;
};
