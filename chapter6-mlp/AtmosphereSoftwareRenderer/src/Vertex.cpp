#include "Vertex.h"

Vertex::Vertex(const glm::vec4 & pos, const glm::vec4 & texcoords, const glm::vec4 & normal)
{
    m_pos = pos;
    m_texcoords = texcoords;
    m_normal = normal;
}

float Vertex::triangleArea(Vertex b, Vertex c) const
{
    float x1 = b.m_pos.x - m_pos.x;
    float y1 = b.m_pos.y - m_pos.y;

    float x2 = c.m_pos.x - m_pos.x;
    float y2 = c.m_pos.y - m_pos.y;

    return 0.5f * (x1 * y2 - x2 * y1);
}

Vertex Vertex::transform(const glm::mat4 & transform, const glm::mat4 & normal_transform) const
{
   return Vertex(transform * m_pos, m_texcoords, normal_transform * m_normal);
}

Vertex Vertex::perspectiveDivide() const
{
    glm::vec4 v { m_pos.x / m_pos.w, 
                  m_pos.y / m_pos.w, 
                  m_pos.z / m_pos.w, 
                  m_pos.w };

    return Vertex(v, m_texcoords, m_normal);
}

Vertex Vertex::lerp(const Vertex& v, float amt) const
{
    return Vertex(glm::mix(m_pos, v.m_pos, amt),
                  glm::mix(m_texcoords, v.m_texcoords, amt),
                  glm::mix(m_normal, v.m_normal, amt));
}
