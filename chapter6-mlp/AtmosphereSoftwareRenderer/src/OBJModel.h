#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

class OBJModel
{
public:
    explicit OBJModel(const std::string& file_name);

    std::vector<int> m_indices;
    std::vector<glm::vec4> m_positions;
    std::vector<glm::vec4> m_normals;
    std::vector<glm::vec4> m_tangents;
    std::vector<glm::vec4> m_texcoords;

    bool isGood() const { return m_is_good; }

private:
    bool m_is_good;
    void calcNormals();
    void calcTangents();
};
